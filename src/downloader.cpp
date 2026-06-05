#include "../include/downloader.hpp"
#include <iostream>
#include <fstream>
#include <thread>
#include <curl/curl.h>

// ─────────────────────────────────────────
// CURL write callback
// ─────────────────────────────────────────
size_t Downloader::writeData(void* buffer,
                             size_t size,
                             size_t nmemb,
                             void* stream) {

    std::ofstream* file = static_cast<std::ofstream*>(stream);
    file->write((char*)buffer, size * nmemb);
    return size * nmemb;
}

// ─────────────────────────────────────────
// Download one chunk
// ─────────────────────────────────────────
void Downloader::downloadChunk(const std::string& url,
                               const ChunkInfo& chunk) {

    CURL* curl = curl_easy_init();

    if (!curl) {
        std::cerr << "❌ Thread " << chunk.threadId
                  << " failed to init curl\n";
        return;
    }

    // Open temp file
    std::ofstream file(chunk.tempFile, std::ios::binary);

    if (!file) {
        std::cerr << "❌ Cannot open file: "
                  << chunk.tempFile << "\n";
        curl_easy_cleanup(curl);
        return;
    }

    // Range string e.g. "bytes=0-999"
    std::string range =
        "bytes=" + std::to_string(chunk.startByte) +
        "-" + std::to_string(chunk.endByte);

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeData);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &file);
    curl_easy_setopt(curl, CURLOPT_RANGE, range.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    std::cout << "⬇️ Thread " << chunk.threadId
              << " downloading (" << range << ")\n";

    //CURLcode res = curl_easy_perform(curl);

   // if (res != CURLE_OK) {
    //    std::cerr << "❌ Thread " << chunk.threadId
    //              << " error: "
    //              << curl_easy_strerror(res) << "\n";
   // } else {
   //     std::cout << "✅ Thread " << chunk.threadId
   //               << " finished\n";
    //}

   // file.close();
    //curl_easy_cleanup(curl);
    
}

// ─────────────────────────────────────────
// Launch all threads
// ─────────────────────────────────────────
void Downloader::downloadAll(const std::string& url,
                             const std::vector<ChunkInfo>& chunks) {

    std::cout << "\n================================\n";
    std::cout << "       DOWNLOADING FILE         \n";
    std::cout << "================================\n";

    std::vector<std::thread> threads;

    // Create threads
    for (const auto& chunk : chunks) {
        threads.emplace_back(downloadChunk, url, chunk);
    }

    // Join threads
    for (auto& t : threads) {
        t.join();
    }

    std::cout << "================================\n";
    std::cout << "✅ All chunks downloaded\n";
    std::cout << "================================\n";
}