#include "../include/validator.hpp"
#include <iostream>
#include <regex>

// ─────────────────────────────────────────
// Check URL starts with http:// or https://
// ─────────────────────────────────────────
bool Validator::isValidUrl(const std::string& url) {

    if (url.empty()) {
        std::cerr << "❌ Error: URL cannot be empty\n";
        return false;
    }

    // Simple regex for http/https URL
    std::regex urlPattern(
        R"(https?:\/\/(www\.)?[-a-zA-Z0-9@:%._\+~#=]{1,256})"
        R"(\.[a-zA-Z0-9()]{1,6}\b([-a-zA-Z0-9()@:%_\+.~#?&\/=]*))"
    );

    if (!std::regex_match(url, urlPattern)) {
        std::cerr << "❌ Error: Invalid URL format\n";
        std::cerr << "   Use: http://example.com/file.zip\n";
        return false;
    }

    return true;
}

// ─────────────────────────────────────────
// Thread count must be between 1 and 16
// ─────────────────────────────────────────
bool Validator::isValidThreadCount(int threads) {

    if (threads < 1 || threads > 16) {
        std::cerr << "❌ Error: Thread count must be 1-16\n";
        std::cerr << "   You entered: " << threads << "\n";
        return false;
    }

    return true;
}

// ─────────────────────────────────────────
// Output name must not be empty or have
// illegal characters
// ─────────────────────────────────────────
bool Validator::isValidOutputName(const std::string& name) {

    if (name.empty()) {
        std::cerr << "❌ Error: Output filename cannot be empty\n";
        return false;
    }

    // Check for illegal characters
    std::string illegal = "/\\:*?\"<>|";
    for (char c : illegal) {
        if (name.find(c) != std::string::npos) {
            std::cerr << "❌ Error: Filename has illegal character: "
                      << c << "\n";
            return false;
        }
    }

    return true;
}

// ─────────────────────────────────────────
// Run all 3 validations at once
// ─────────────────────────────────────────
bool Validator::validateAll(const std::string& url,
                             int threads,
                             const std::string& outputName) {

    std::cout << "\n================================\n";
    std::cout << "       INPUT VALIDATION         \n";
    std::cout << "================================\n";

    bool urlOk     = isValidUrl(url);
    bool threadOk  = isValidThreadCount(threads);
    bool outputOk  = isValidOutputName(outputName);

    if (urlOk)     std::cout << "✅ URL      : " << url << "\n";
    if (threadOk)  std::cout << "✅ Threads  : " << threads << "\n";
    if (outputOk)  std::cout << "✅ Output   : " << outputName << "\n";

    std::cout << "================================\n";

    return urlOk && threadOk && outputOk;
}