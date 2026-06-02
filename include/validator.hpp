#pragma once
#include <string>

// ─────────────────────────────────────────
// Validates user input before any download
// ─────────────────────────────────────────
class Validator {
public:

    // Check if URL format is valid
    static bool isValidUrl(const std::string& url);

    // Check if thread count is in range 1-16
    static bool isValidThreadCount(int threads);

    // Check if output filename is valid
    static bool isValidOutputName(const std::string& name);

    // Run all validations together
    // Returns true only if everything is valid
    static bool validateAll(const std::string& url,
                            int threads,
                            const std::string& outputName);
};