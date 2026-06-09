#pragma once

#include <atomic>
#include <functional>
#include <fstream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "chunkcalculator.hpp"
#include "filemerger.hpp"
#include "progresstracker.hpp"

class ThreadManager {
public:
    void createThreads(const std::vector<std::function<void()>>& tasks);
    void joinThreads();

private:
    std::vector<std::thread> threads;
};

class TempFileHandler {
public:
    bool cleanupTempFiles(const std::vector<ChunkInfo>& chunks) const;
};

class Downloader {
public:
    Downloader(const std::string& url,
               const ChunkInfo& chunk,
               ProgressTracker& tracker);

    bool downloadChunk(int maxRetries = 3);

private:
    static size_t writeCallback(void* ptr,
                                size_t size,
                                size_t nmemb,
                                void* userData);

    std::string url;
    ChunkInfo chunk;
    ProgressTracker* tracker;
};

class DownloadStrategy {
public:
    virtual ~DownloadStrategy() = default;
    virtual bool download(const std::string& url,
                          const std::vector<ChunkInfo>& chunks,
                          const std::string& outputName,
                          ProgressTracker::ProgressCallback progressCallback = {}) = 0;
};

class SingleThreadDownloader : public DownloadStrategy {
public:
    bool download(const std::string& url,
                  const std::vector<ChunkInfo>& chunks,
                  const std::string& outputName,
                  ProgressTracker::ProgressCallback progressCallback = {}) override;
};

class MultiThreadDownloader : public DownloadStrategy {
public:
    bool download(const std::string& url,
                  const std::vector<ChunkInfo>& chunks,
                  const std::string& outputName,
                  ProgressTracker::ProgressCallback progressCallback = {}) override;
};

class DownloaderFactory {
public:
    static std::unique_ptr<DownloadStrategy> createDownloader(int threadCount);
};

// Global cancellation control (simple cross-module token)
void setGlobalCancel(bool value);
bool isGlobalCancelled();
