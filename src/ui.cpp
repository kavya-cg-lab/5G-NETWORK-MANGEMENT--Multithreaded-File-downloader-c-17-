#include "../include/ui.hpp"
#include <cmath>
#include <iostream>
#include <iomanip>

static const char* resetColor  = "\033[0m";
static const char* greenColor  = "\033[32m";
static const char* yellowColor = "\033[33m";
static const char* blueColor   = "\033[34m";
static const char* redColor    = "\033[31m";

void UIManager::showWelcome() {
    std::cout << greenColor
              << "=============================\n"
              << " MULTITHREADED FILE DOWNLOADER\n"
              << "=============================\n"
              << resetColor;
}

void UIManager::showHelp() {
    std::cout << "Usage:\n";
    std::cout << "  ./downloader <url> <output-file>\n";
    std::cout << "  ./downloader <url> <threads> <output-file>\n";
    std::cout << "\nExamples:\n";
    std::cout << "  ./downloader http://speedtest.tele2.net/100MB.zip output.zip\n";
    std::cout << "  ./downloader http://speedtest.tele2.net/1GB.zip 8 output.zip\n";
    std::cout << "\nNotes:\n";
    std::cout << "  - threads is optional; auto-selected from file size if omitted\n";
    std::cout << "  - if given, threads is used as the maximum (1-16)\n";
}

void UIManager::showStepStart(const std::string& stepName) {
    std::cout << blueColor << "\n[" << stepName << "] " << resetColor << "\n";
}

void UIManager::showStepStatus(const std::string& status) {
    std::cout << "  " << status << "\n";
}

void UIManager::showRetry(int attempt, int maxAttempts) {
    std::cout << yellowColor
              << "  Retry " << attempt << " of " << maxAttempts << "...\n"
              << resetColor;
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
    for (int i = 0; i < barWidth; ++i)
        std::cout << (i < pos ? '#' : '-');
    std::cout << "] "
              << std::fixed << std::setprecision(1) << (progress * 100.0) << "% "
              << "| " << speedKBps << " KB/s "
              << "| ETA " << etaSeconds << "s"
              << std::flush;
}

void UIManager::showFinalResult(bool success, const std::string& message) {
    std::cout << "\n";
    if (success)
        std::cout << greenColor << "✅ " << message << resetColor << "\n";
    else
        std::cout << redColor   << "❌ " << message << resetColor << "\n";
}