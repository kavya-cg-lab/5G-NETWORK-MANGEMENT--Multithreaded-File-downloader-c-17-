#pragma once
#include <string>
#include <vector>
#include "chunkcalculator.hpp"

// ─────────────────────────────────────────
// Handles multithreaded downloading
// ─────────────────────────────────────────
class Downloader {
public:

    // Download all chunks using threads
    static void downloadAll(const std::string& url,
                            const std::vector<ChunkInfo>& chunks);

private:

    // Worker function for one thread
    static void downloadChunk(const std::string& url,
                              const ChunkInfo& chunk);

    // Write callback for curl → writes to file
    static size_t writeData(void* buffer,
                             size_t size,
                             size_t nmemb,
                             void* stream);
};