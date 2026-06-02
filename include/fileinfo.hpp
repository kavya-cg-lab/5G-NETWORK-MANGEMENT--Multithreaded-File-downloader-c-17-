#pragma once
#include <string>

// ─────────────────────────────────────────
// Holds all info fetched from the server
// ─────────────────────────────────────────
struct FileData {
    long long fileSize    = 0;
    bool supportsRange    = false;
    bool isValid          = false;
    std::string fileName;
};

// ─────────────────────────────────────────
// Fetches file metadata from server
// using HTTP HEAD request via libcurl
// ─────────────────────────────────────────
class FileInfo {
public:

    // Main function — send HEAD request
    // and return file metadata
    static FileData fetch(const std::string& url);

    // Print file info to terminal
    static void print(const FileData& data);

private:

    // Curl write callback — discards body
    // data (we only need headers here)
    static size_t discardData(void* buffer,
                               size_t size,
                               size_t nmemb,
                               void* userp);
};