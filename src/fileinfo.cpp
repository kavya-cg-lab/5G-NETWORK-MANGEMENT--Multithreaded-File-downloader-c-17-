#include "../include/fileinfo.hpp"
#include <iostream>
#include <curl/curl.h>

// ─────────────────────────────────────────
// Discard response body — we only need
// the headers for file size info
// ─────────────────────────────────────────
size_t FileInfo::discardData(void* buffer,
                              size_t size,
                              size_t nmemb,
                              void* userp) {
    return size * nmemb;
}

// ─────────────────────────────────────────
// Send HTTP HEAD request to get:
//  - File size (Content-Length)
//  - Range support (Accept-Ranges)
// ─────────────────────────────────────────
FileData FileInfo::fetch(const std::string& url) {

    FileData data;
    CURL* curl = curl_easy_init();

    if (!curl) {
        std::cerr << "❌ Failed to initialize curl\n";
        return data;
    }

    // ── First request: get file size ──
    curl_easy_setopt(curl, CURLOPT_URL,           url.c_str());
    curl_easy_setopt(curl, CURLOPT_NOBODY,         1L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT,        30L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,  discardData);

    CURLcode result = curl_easy_perform(curl);

    if (result != CURLE_OK) {
        std::cerr << "❌ Curl error: "
                  << curl_easy_strerror(result) << "\n";
        curl_easy_cleanup(curl);
        return data;
    }

    // Get content length from response
    curl_off_t fileSize = 0;
    curl_easy_getinfo(curl,
                      CURLINFO_CONTENT_LENGTH_DOWNLOAD_T,
                      &fileSize);

    // ── Second request: check Range support ──
    curl_easy_reset(curl);
    curl_easy_setopt(curl, CURLOPT_URL,           url.c_str());
    curl_easy_setopt(curl, CURLOPT_NOBODY,         1L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,  discardData);

    // Send Range header to test if server
    // responds with HTTP 206 Partial Content
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Range: bytes=0-1");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    curl_easy_perform(curl);

    long httpCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

    // HTTP 206 = server supports partial download
    data.supportsRange = (httpCode == 206);
    data.fileSize      = (long long)fileSize;
    data.isValid       = (fileSize > 0);

    // Extract filename from URL

    size_t lastSlash = url.find_last_of('/');

    if (lastSlash != std::string::npos &&
        lastSlash + 1 < url.size()) {

        data.fileName = url.substr(lastSlash + 1);

    // If substring is empty → fallback
        if (data.fileName.empty()) {
            data.fileName = "downloaded_file";
        }

    } else {
        data.fileName = "downloaded_file";
    }
 
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return data;
}

// ─────────────────────────────────────────
// Print fetched file info to terminal
// ─────────────────────────────────────────
void FileInfo::print(const FileData& data) {

    std::cout << "\n================================\n";
    std::cout << "       FILE INFORMATION         \n";
    std::cout << "================================\n";

    if (!data.isValid) {
        std::cout << "❌ Could not reach server\n";
        std::cout << "================================\n";
        return;
    }

    std::cout << "✅ File Name  : " << data.fileName  << "\n";
    std::cout << "✅ File Size  : " << data.fileSize
              << " bytes\n";
    std::cout << "            : " << data.fileSize / 1024
              << " KB\n";
    std::cout << "            : " << data.fileSize / (1024 * 1024)
              << " MB\n";
    std::cout << "✅ Range      : "
              << (data.supportsRange ? "Supported ✅"
                                     : "Not Supported ❌")
              << "\n";
    std::cout << "✅ Threading  : "
              << (data.supportsRange ? "Multithreaded ✅"
                                     : "Single Thread only")
              << "\n";
    std::cout << "================================\n";
}