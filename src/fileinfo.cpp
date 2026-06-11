#include "../include/fileinfo.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <curl/curl.h>

size_t FileInfo::discardData(void* /*buffer*/,
                              size_t size, size_t nmemb,
                              void* /*userp*/) {
    return size * nmemb;
}

// ─────────────────────────────────────────
// fetch
//
// FIX v2:
// Request 1 — GET (not HEAD) with FOLLOWLOCATION ON and Range: bytes=0-1
//   Some CDN servers (e.g. tele2 speedtest) ignore or mis-handle HEAD
//   requests for range negotiation. A GET with Range: bytes=0-1 is the
//   only reliable way to simultaneously:
//     a) follow all redirects to find the final CDN URL
//     b) get a 206 response that confirms range support
//     c) read Content-Range to discover the true file size
//
// This single request replaces the previous two-request approach which
// caused HTTP 416 because the resolved URL from the HEAD request did
// not match the URL that actually served the ranged content.
// ─────────────────────────────────────────
FileData FileInfo::fetch(const std::string& url) {

    FileData data;

    // ── Single request: GET with Range: bytes=0-1, follow redirects ──
    CURL* curl = curl_easy_init();
    if (!curl) { std::cerr << "Failed to init curl\n"; return data; }

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Range: bytes=0-1");

    // Buffer to capture response headers
    std::string responseHeaders;

    auto headerCallback = [](char* buf, size_t size, size_t nitems, void* userdata) -> size_t {
        auto* hdrs = static_cast<std::string*>(userdata);
        hdrs->append(buf, size * nitems);
        return size * nitems;
    };

    curl_easy_setopt(curl, CURLOPT_URL,            url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER,     headers);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS,      20L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT,        30L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 15L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT,      "Mozilla/5.0");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,  discardData);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, +headerCallback);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA,     &responseHeaders);

    CURLcode rc = curl_easy_perform(curl);

    if (rc != CURLE_OK) {
        std::cerr << "curl GET error: " << curl_easy_strerror(rc) << "\n";
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return data;
    }

    long httpCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

    // Capture final resolved URL (after all redirects)
    char* effective = nullptr;
    curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &effective);
    data.resolvedUrl = (effective && *effective) ? effective : url;

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    // ── Parse Content-Range header for true file size ──────────────
    // Content-Range: bytes 0-1/104857600
    long long fileSize = 0;
    bool      supportsRange = false;

    if (httpCode == 206) {
        supportsRange = true;

        // Find "Content-Range:" in response headers (case-insensitive search)
        std::string hdrsLower = responseHeaders;
        // Search for content-range header
        size_t pos = std::string::npos;
        // Try different capitalisations
        for (const char* key : {"Content-Range:", "content-range:", "CONTENT-RANGE:"}) {
            pos = responseHeaders.find(key);
            if (pos != std::string::npos) break;
        }

        if (pos != std::string::npos) {
            // Move past "Content-Range: bytes 0-1/"
            size_t slashPos = responseHeaders.find('/', pos);
            if (slashPos != std::string::npos) {
                size_t numStart = slashPos + 1;
                size_t numEnd   = responseHeaders.find_first_not_of("0123456789", numStart);
                if (numEnd == std::string::npos) numEnd = responseHeaders.size();
                std::string numStr = responseHeaders.substr(numStart, numEnd - numStart);
                if (!numStr.empty()) {
                    try { fileSize = std::stoll(numStr); } catch (...) { fileSize = 0; }
                }
            }
        }
    } else if (httpCode == 200) {
        // Server delivered full file instead of 206 — range not supported
        supportsRange = false;

        // Try to get size from Content-Length header
        size_t pos = responseHeaders.find("Content-Length:");
        if (pos == std::string::npos) pos = responseHeaders.find("content-length:");
        if (pos != std::string::npos) {
            size_t valStart = responseHeaders.find_first_of("0123456789", pos);
            if (valStart != std::string::npos) {
                size_t valEnd = responseHeaders.find_first_not_of("0123456789", valStart);
                std::string numStr = responseHeaders.substr(valStart,
                    valEnd == std::string::npos ? std::string::npos : valEnd - valStart);
                try { fileSize = std::stoll(numStr); } catch (...) { fileSize = 0; }
            }
        }
    } else {
        std::cerr << "Unexpected HTTP " << httpCode
                  << " from " << data.resolvedUrl << "\n";
        return data;
    }

    // If Content-Range parsing failed, fall back to a HEAD request for size
    if (fileSize <= 0 && supportsRange) {
        curl = curl_easy_init();
        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL,            data.resolvedUrl.c_str());
            curl_easy_setopt(curl, CURLOPT_NOBODY,         1L);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L);
            curl_easy_setopt(curl, CURLOPT_TIMEOUT,        20L);
            curl_easy_setopt(curl, CURLOPT_USERAGENT,      "Mozilla/5.0");
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,  discardData);
            curl_easy_perform(curl);
            curl_off_t sz = 0;
            curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &sz);
            if (sz > 0) fileSize = static_cast<long long>(sz);
            curl_easy_cleanup(curl);
        }
    }

    data.supportsRange = supportsRange;
    data.fileSize      = fileSize;
    data.isValid       = (fileSize > 0);

    // Extract filename from original URL
    size_t slash = url.find_last_of('/');
    data.fileName = (slash != std::string::npos && slash + 1 < url.size())
                    ? url.substr(slash + 1)
                    : "downloaded_file";
    // Strip query string
    size_t qmark = data.fileName.find('?');
    if (qmark != std::string::npos) data.fileName = data.fileName.substr(0, qmark);
    if (data.fileName.empty()) data.fileName = "downloaded_file";

    return data;
}

// ─────────────────────────────────────────
// FILE INFORMATION box
// ─────────────────────────────────────────
void FileInfo::print(const FileData& data, int numThreads) {

    auto row = [](const std::string& label, const std::string& val) {
        std::ostringstream s;
        s << "│  " << std::left << std::setw(16) << label
          << " : " << std::setw(32) << val << "│\n";
        std::cout << s.str();
    };

    std::cout
        << "\n┌─────────────────────────────────────────────────────┐\n"
        << "│              FILE INFORMATION                       │\n"
        << "├─────────────────────────────────────────────────────┤\n";

    if (!data.isValid) {
        std::cout
            << "│  Could not reach server                             │\n"
            << "└─────────────────────────────────────────────────────┘\n";
        return;
    }

    row("File Name",    data.fileName);
    row("Size (bytes)", std::to_string(data.fileSize));

    std::ostringstream mb;
    mb << std::fixed << std::setprecision(2)
       << (data.fileSize / (1024.0 * 1024.0)) << " MB";
    row("Size (MB)",     mb.str());
    row("Range Support", data.supportsRange ? "Yes \u2713" : "No");
    row("Threads",       std::to_string(numThreads));

    std::cout
        << "└─────────────────────────────────────────────────────┘\n";
}