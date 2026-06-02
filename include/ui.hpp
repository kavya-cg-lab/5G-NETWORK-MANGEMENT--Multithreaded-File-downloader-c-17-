#pragma once

#include <string>

class UIManager {
public:
    static void showWelcome();
    static void showHelp();
    static void showStepStart(const std::string& stepName);
    static void showStepStatus(const std::string& status);
    static void showRetry(int attempt, int maxAttempts);
    static void showProgress(long long downloadedBytes,
                             long long totalBytes,
                             double speedKBps,
                             int etaSeconds);
    static void showFinalResult(bool success,
                                const std::string& message);
};
