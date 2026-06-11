#pragma once
#include <string>
#include <sstream>

// ─────────────────────────────────────────
// Holds all info fetched from the server
// ─────────────────────────────────────────
struct FileData {
    long long   fileSize      = 0;
    bool        supportsRange = false;
    bool        isValid       = false;
    std::string fileName;
    std::string resolvedUrl;   // final URL after all redirects
};

// ─────────────────────────────────────────
// Fetches file metadata via a ranged GET
// (single request that resolves redirects
//  AND confirms range support in one shot)
// ─────────────────────────────────────────
class FileInfo {
public:
    static FileData fetch(const std::string& url);

    // numThreads shown in FILE INFORMATION box
    static void print(const FileData& data, int numThreads);

private:
    static size_t discardData(void*, size_t, size_t, void*);
};