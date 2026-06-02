#include "../include/downloadsystem.hpp"
#include <curl/curl.h>
#include <fstream>
#include <iostream>
#include <cstdio>

struct WriteContext {
    std::ofstream* output;
    ProgressTracker* progress;
};

void ThreadManager::createThreads(const std::vector<std::function<void()>>& tasks) {
    threads.clear();
    threads.reserve(tasks.size());
    for (const auto& task : tasks) {
        threads.emplace_back(task);
    }
}

void ThreadManager::joinThreads() {
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

bool TempFileHandler::cleanupTempFiles(const std::vector<ChunkInfo>& chunks) const {
    bool success = true;

    for (const auto& chunk : chunks) {
        if (std::remove(chunk.tempFile.c_str()) != 0) {
            std::cerr << "⚠️  Could not delete temp file: "
                      << chunk.tempFile << "\n";
            success = false;
        }
    }

    return success;
}

Downloader::Downloader(const std::string& url,
                       const ChunkInfo& chunk,
                       ProgressTracker& tracker)
    : url(url),
      chunk(chunk),
      tracker(&tracker) {
}

bool Downloader::downloadChunk(int maxRetries) {
    for (int attempt = 1; attempt <= maxRetries; ++attempt) {
        std::ofstream outputFile(chunk.tempFile, std::ios::binary);
        if (!outputFile) {
            std::cerr << "❌ Failed to create chunk file: "
                      << chunk.tempFile << "\n";
            return false;
        }

        CURL* curl = curl_easy_init();
        if (!curl) {
            std::cerr << "❌ curl_easy_init failed\n";
            return false;
        }

        std::string rangeSpec = std::to_string(chunk.startByte)
                              + "-"
                              + std::to_string(chunk.endByte);

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        WriteContext context{&outputFile, tracker};
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &context);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 120L);
        curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 30L);
        curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 1L);

        if (chunk.startByte != 0 || chunk.endByte != -1) {
            curl_easy_setopt(curl, CURLOPT_RANGE, rangeSpec.c_str());
        }

        CURLcode result = curl_easy_perform(curl);
        bool ok = (result == CURLE_OK);

        if (ok) {
            curl_easy_cleanup(curl);
            return true;
        }

        std::cerr << "❌ Curl failed for chunk "
                  << chunk.threadId << " attempt " << attempt
                  << ": " << curl_easy_strerror(result) << "\n";

        curl_easy_cleanup(curl);

        if (attempt < maxRetries) {
            std::cerr << "   Retrying chunk " << chunk.threadId
                      << " (" << attempt + 1 << "/" << maxRetries << ")...\n";
        }
    }

    return false;
}

size_t Downloader::writeCallback(void* ptr,
                                 size_t size,
                                 size_t nmemb,
                                 void* userData) {
    auto* context = static_cast<WriteContext*>(userData);
    size_t totalSize = size * nmemb;

    context->output->write(static_cast<char*>(ptr), totalSize);
    if (!*context->output) {
        return 0;
    }

    context->progress->addBytes(totalSize);
    return totalSize;
}

bool SingleThreadDownloader::download(const std::string& url,
                                      const std::vector<ChunkInfo>& chunks,
                                      const std::string& outputName) {
    (void)outputName;
    if (chunks.empty()) {
        std::cerr << "❌ No chunks available for single-threaded download.\n";
        return false;
    }

    ProgressTracker tracker(chunks.front().chunkSize);
    Downloader downloader(url, chunks.front(), tracker);
    bool success = downloader.downloadChunk();
    tracker.printFinal();
    return success;
}

bool MultiThreadDownloader::download(const std::string& url,
                                     const std::vector<ChunkInfo>& chunks,
                                     const std::string& outputName) {
    if (chunks.empty()) {
        std::cerr << "❌ No chunks available for multi-threaded download.\n";
        return false;
    }

    long long totalBytes = 0;
    for (const auto& chunk : chunks) {
        totalBytes += chunk.chunkSize;
    }

    ProgressTracker tracker(totalBytes);
    std::vector<bool> results(chunks.size(), false);
    std::vector<std::function<void()>> tasks;
    tasks.reserve(chunks.size());

    for (size_t index = 0; index < chunks.size(); ++index) {
        ChunkInfo chunk = chunks[index];
        tasks.emplace_back([&, index, chunk]() {
            Downloader downloader(url, chunk, tracker);
            results[index] = downloader.downloadChunk();
        });
    }

    ThreadManager manager;
    manager.createThreads(tasks);
    manager.joinThreads();

    bool allSucceeded = true;
    for (size_t i = 0; i < results.size(); ++i) {
        if (!results[i]) {
            std::cerr << "❌ Chunk " << i << " failed.\n";
            allSucceeded = false;
        }
    }

    if (!allSucceeded) {
        return false;
    }

    tracker.printFinal();
    FileMerger merger;
    if (!merger.mergeFiles(chunks, outputName)) {
        return false;
    }

    TempFileHandler cleaner;
    cleaner.cleanupTempFiles(chunks);
    return true;
}

std::unique_ptr<DownloadStrategy>
DownloaderFactory::createDownloader(int threadCount) {
    if (threadCount <= 1) {
        return std::make_unique<SingleThreadDownloader>();
    }

    return std::make_unique<MultiThreadDownloader>();
}
