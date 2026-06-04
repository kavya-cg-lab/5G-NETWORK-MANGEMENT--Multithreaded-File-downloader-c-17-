#pragma once

#include <atomic>
#include <chrono>
#include <functional>
#include <mutex>
#include <string>

class ProgressTracker {
public:
    explicit ProgressTracker(long long totalBytes);

    void addBytes(size_t bytes);
    void printFinal() const;
    
    // Set a callback for progress updates (for GUI integration)
    using ProgressCallback = std::function<void(long long, long long, double, int)>;
    void setProgressCallback(ProgressCallback callback) {
        progressCallback = callback;
    }

private:
    long long totalBytes;
    std::atomic<long long> downloadedBytes;
    std::atomic<long long> nextReport;
    std::chrono::steady_clock::time_point startTime;
    std::mutex printMutex;
    static constexpr long long reportStep = 1024 * 50;
    
    ProgressCallback progressCallback;
};
