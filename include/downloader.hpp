#pragma once
#include <string>
#include <vector>
#include <mutex>
#include "chunkcalculator.hpp"

class Downloader {
public:
    // resolvedUrl must be the final URL after all redirects
    // (obtained from FileData::resolvedUrl)
    static bool downloadAll(const std::string& resolvedUrl,
                            const std::vector<ChunkInfo>& chunks);

private:
    static void   downloadChunk(const std::string& resolvedUrl,
                                 const ChunkInfo& chunk);
    static size_t writeData(void* buf, size_t size, size_t nmemb,
                             void* stream);
    static std::vector<std::string> threadErrors_;
    static std::mutex               errorMutex_;
};