#pragma once
#include <string>

// ─────────────────────────────────────────
// Validates user-supplied inputs before
// any network or filesystem work begins
// ─────────────────────────────────────────
class Validator {
public:

    // Returns true if URL starts with http:// or https://
    // and contains at least one '.' after the scheme
    static bool isValidUrl(const std::string& url);

    // Returns true if numThreads is between 1 and maxAllowed
    static bool isValidThreadCount(int numThreads,
                                   int maxAllowed = 64);

    // Returns true if fileSize is strictly positive
    static bool isValidFileSize(long long fileSize);

    // Print a human-readable error for a bad URL
    static void printUrlError(const std::string& url);

    // Print a human-readable error for a bad thread count
    static void printThreadCountError(int numThreads,
                                      int maxAllowed = 64);
};