#pragma once

#include <atomic>
#include <chrono>
#include <mutex>
#include <string>

class ProgressTracker {
public:
    explicit ProgressTracker(long long totalBytes);

    void addBytes(size_t bytes);
    void printFinal() const;

private:
    long long totalBytes;
    std::atomic<long long> downloadedBytes;
    std::atomic<long long> nextReport;
    std::chrono::steady_clock::time_point startTime;
    std::mutex printMutex;
    static constexpr long long reportStep = 1024 * 50;
};
