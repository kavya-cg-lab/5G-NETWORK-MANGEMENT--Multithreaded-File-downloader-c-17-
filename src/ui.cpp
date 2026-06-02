#include "../include/ui.hpp"
#include <cmath>
#include <iostream>
#include <iomanip>

static const char* resetColor = "\033[0m";
static const char* greenColor = "\033[32m";
static const char* yellowColor = "\033[33m";
static const char* blueColor = "\033[34m";
static const char* redColor = "\033[31m";

void UIManager::showWelcome() {
    std::cout << greenColor;
    std::cout << "=============================\n";
    std::cout << " MULTITHREADED FILE DOWNLOADER\n";
    std::cout << "=============================\n";
    std::cout << resetColor;
}

void UIManager::showHelp() {
    std::cout << "Usage: downloader <url> <threads> <output-file>\n";
    std::cout << "Example: ./downloader http://speedtest.tele2.net/100MB.zip 8 largefile.zip\n";
}

void UIManager::showStepStart(const std::string& stepName) {
    std::cout << blueColor << "\n[" << stepName << "] " << resetColor << "\n";
}

void UIManager::showStepStatus(const std::string& status) {
    std::cout << "  " << status << "\n";
}

void UIManager::showRetry(int attempt, int maxAttempts) {
    std::cout << yellowColor;
    std::cout << "  Retry " << attempt << " of " << maxAttempts << "...\n";
    std::cout << resetColor;
}

void UIManager::showProgress(long long downloadedBytes,
                             long long totalBytes,
                             double speedKBps,
                             int etaSeconds) {
    int barWidth = 40;
    double progress = totalBytes > 0
                      ? downloadedBytes / static_cast<double>(totalBytes)
                      : 0.0;
    int pos = static_cast<int>(std::round(barWidth * progress));

    std::cout << "\r[";
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) std::cout << "#";
        else std::cout << "-";
    }
    std::cout << "] ";
    std::cout << std::fixed << std::setprecision(1)
              << (progress * 100.0) << "% ";
    std::cout << "| " << speedKBps << " KB/s ";
    std::cout << "| ETA " << etaSeconds << "s";
    std::cout << std::flush;
}

void UIManager::showFinalResult(bool success,
                                const std::string& message) {
    std::cout << "\n";
    if (success) {
        std::cout << greenColor << "✅ " << message << resetColor << "\n";
    } else {
        std::cout << redColor << "❌ " << message << resetColor << "\n";
    }
}
