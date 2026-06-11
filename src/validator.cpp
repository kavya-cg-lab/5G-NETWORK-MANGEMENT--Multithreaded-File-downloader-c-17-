#include "../include/validator.hpp"
#include <iostream>

// ─────────────────────────────────────────
// URL must start with http:// or https://
// and have content after the scheme+host
// ─────────────────────────────────────────
bool Validator::isValidUrl(const std::string& url) {

    if (url.empty()) return false;

    bool hasScheme = (url.rfind("http://",  0) == 0 ||
                      url.rfind("https://", 0) == 0);
    if (!hasScheme) return false;

    // Must have a dot somewhere after the scheme
    size_t schemeEnd = url.find("//") + 2;
    size_t dotPos    = url.find('.', schemeEnd);
    if (dotPos == std::string::npos) return false;

    // Must have something after the dot
    if (dotPos + 1 >= url.size()) return false;

    return true;
}

// ─────────────────────────────────────────
// Thread count must be in [1, maxAllowed]
// ─────────────────────────────────────────
bool Validator::isValidThreadCount(int numThreads,
                                   int maxAllowed) {
    return (numThreads >= 1 && numThreads <= maxAllowed);
}

// ─────────────────────────────────────────
// File size must be positive
// ─────────────────────────────────────────
bool Validator::isValidFileSize(long long fileSize) {
    return fileSize > 0;
}

// ─────────────────────────────────────────
// Error messages
// ─────────────────────────────────────────
void Validator::printUrlError(const std::string& url) {
    std::cerr << "Invalid URL: \"" << url << "\"\n"
              << "URL must start with http:// or https:// "
                 "and contain a valid hostname.\n";
}

void Validator::printThreadCountError(int numThreads,
                                      int maxAllowed) {
    std::cerr << "Invalid thread count: " << numThreads << "\n"
              << "Must be between 1 and " << maxAllowed << ".\n";
}