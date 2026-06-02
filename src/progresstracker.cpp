#include "../include/progresstracker.hpp"
#include "../include/ui.hpp"
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>

ProgressTracker::ProgressTracker(long long totalBytes)
    : totalBytes(totalBytes),
      downloadedBytes(0),
      nextReport(reportStep),
      startTime(std::chrono::steady_clock::now()) {
}

void ProgressTracker::addBytes(size_t bytes) {
    long long current = downloadedBytes.fetch_add(bytes) + static_cast<long long>(bytes);
    if (current >= nextReport.load()) {
        std::lock_guard<std::mutex> lock(printMutex);
        if (current >= nextReport.load()) {
            auto now = std::chrono::steady_clock::now();
            std::chrono::duration<double> elapsed = now - startTime;
            double seconds = std::max(elapsed.count(), 0.001);
            double speed = current / seconds; // bytes per second

            double remaining = totalBytes - current;
            double etaSeconds = remaining > 0 ? remaining / speed : 0.0;

            UIManager::showProgress(current,
                                    totalBytes,
                                    speed / 1024.0,
                                    static_cast<int>(etaSeconds));
            nextReport = current + reportStep;
        }
    }
}

void ProgressTracker::printFinal() const {
    auto now = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed = now - startTime;
    double seconds = std::max(elapsed.count(), 0.001);
    double speed = totalBytes / seconds;

    std::cout << "\rDownloading: 100% (" << totalBytes << " / "
              << totalBytes << " bytes)"
              << " | Avg " << std::fixed << std::setprecision(1)
              << (speed / 1024.0) << " KB/s"
              << "\n";
}
