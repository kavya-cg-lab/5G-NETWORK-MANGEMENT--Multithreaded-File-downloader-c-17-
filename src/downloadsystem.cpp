#include "../include/downloadsystem.hpp"
#include <curl/curl.h>
#include <fstream>
#include <iostream>
#include <cstdio>

struct WriteContext {
    std::ofstream*   output;
    ProgressTracker* progress;
};

// ── ThreadManager ────────────────────────────────────────────────
void ThreadManager::createThreads(const std::vector<std::function<void()>>& tasks) {
    threads.clear();
    threads.reserve(tasks.size());
    for (const auto& task : tasks) threads.emplace_back(task);
}

void ThreadManager::joinThreads() {
    for (auto& t : threads) if (t.joinable()) t.join();
}

// ── TempFileHandler ──────────────────────────────────────────────
bool TempFileHandler::cleanupTempFiles(const std::vector<ChunkInfo>& chunks) const {
    bool ok = true;
    for (const auto& chunk : chunks) {
        if (std::remove(chunk.tempFile.c_str()) != 0) {
            std::cerr << "⚠️  Could not delete temp file: " << chunk.tempFile << "\n";
            ok = false;
        }
    }
    return ok;
}

// ── Downloader ───────────────────────────────────────────────────
Downloader::Downloader(const std::string& url,
                       const ChunkInfo& chunk,
                       ProgressTracker& tracker)
    : url(url), chunk(chunk), tracker(&tracker) {}

bool Downloader::downloadChunk(int maxRetries) {
    for (int attempt = 1; attempt <= maxRetries; ++attempt) {

        // Truncate/create fresh for each retry so partial data doesn't linger
        std::ofstream outputFile(chunk.tempFile,
                                 std::ios::binary | std::ios::trunc);
        if (!outputFile) {
            std::cerr << "❌ Failed to create chunk file: " << chunk.tempFile << "\n";
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

        curl_easy_setopt(curl, CURLOPT_URL,            url.c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION,  1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,   writeCallback);
        WriteContext context{&outputFile, tracker};
        curl_easy_setopt(curl, CURLOPT_WRITEDATA,       &context);
        // FIX: raise timeout for large chunks (100GB file = ~12GB per thread)
        curl_easy_setopt(curl, CURLOPT_TIMEOUT,         0L);   // no hard timeout
        curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME,  60L);  // stall: 1 min
        curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT,  512L); // below 512 B/s = stall
        curl_easy_setopt(curl, CURLOPT_RANGE,           rangeSpec.c_str());

        CURLcode result = curl_easy_perform(curl);

        // FIX: check HTTP response code — 200/206 = success
        // anything else (301/302 redirect captured as body) = failure
        long httpCode = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
        curl_easy_cleanup(curl);

        bool ok = (result == CURLE_OK) && (httpCode == 200 || httpCode == 206);

        if (ok) return true;

        if (result != CURLE_OK) {
            std::cerr << "❌ Curl failed for chunk " << chunk.threadId
                      << " attempt " << attempt
                      << ": " << curl_easy_strerror(result) << "\n";
        } else {
            std::cerr << "❌ Chunk " << chunk.threadId
                      << " got unexpected HTTP " << httpCode
                      << " (attempt " << attempt << ")\n";
        }

        if (attempt < maxRetries) {
            std::cerr << "   Retrying chunk " << chunk.threadId
                      << " (" << attempt + 1 << "/" << maxRetries << ")...\n";
        }
    }
    return false;
}

size_t Downloader::writeCallback(void* ptr, size_t size, size_t nmemb, void* userData) {
    auto* ctx = static_cast<WriteContext*>(userData);
    size_t total = size * nmemb;
    ctx->output->write(static_cast<char*>(ptr), total);
    if (!*ctx->output) return 0;
    ctx->progress->addBytes(total);
    return total;
}

// ── SingleThreadDownloader ───────────────────────────────────────
bool SingleThreadDownloader::download(const std::string& url,
                                      const std::vector<ChunkInfo>& chunks,
                                      const std::string& outputName,
                                      ProgressTracker::ProgressCallback progressCallback) {
    (void)outputName;
    if (chunks.empty()) {
        std::cerr << "❌ No chunks for single-threaded download.\n";
        return false;
    }
    ProgressTracker tracker(chunks.front().chunkSize);
    tracker.setProgressCallback(std::move(progressCallback));
    Downloader dl(url, chunks.front(), tracker);
    bool ok = dl.downloadChunk();
    tracker.printFinal();
    return ok;
}

// ── MultiThreadDownloader ────────────────────────────────────────
bool MultiThreadDownloader::download(const std::string& url,
                                     const std::vector<ChunkInfo>& chunks,
                                     const std::string& outputName,
                                     ProgressTracker::ProgressCallback progressCallback) {
    if (chunks.empty()) {
        std::cerr << "❌ No chunks for multi-threaded download.\n";
        return false;
    }

    long long totalBytes = 0;
    for (const auto& c : chunks) totalBytes += c.chunkSize;

    ProgressTracker tracker(totalBytes);
    tracker.setProgressCallback(std::move(progressCallback));
    std::vector<bool> results(chunks.size(), false);
    std::vector<std::function<void()>> tasks;
    tasks.reserve(chunks.size());

    for (size_t i = 0; i < chunks.size(); ++i) {
        ChunkInfo chunk = chunks[i];
        tasks.emplace_back([&, i, chunk]() {
            Downloader dl(url, chunk, tracker);
            results[i] = dl.downloadChunk();
        });
    }

    ThreadManager mgr;
    mgr.createThreads(tasks);
    mgr.joinThreads();

    bool allOk = true;
    for (size_t i = 0; i < results.size(); ++i) {
        if (!results[i]) {
            std::cerr << "❌ Chunk " << i << " failed after all retries.\n";
            allOk = false;
        }
    }
    if (!allOk) return false;

    tracker.printFinal();

    FileMerger merger;
    if (!merger.mergeFiles(chunks, outputName)) return false;

    TempFileHandler cleaner;
    cleaner.cleanupTempFiles(chunks);
    return true;
}

// ── DownloaderFactory ────────────────────────────────────────────
std::unique_ptr<DownloadStrategy> DownloaderFactory::createDownloader(int threadCount) {
    if (threadCount <= 1)
        return std::make_unique<SingleThreadDownloader>();
    return std::make_unique<MultiThreadDownloader>();
}