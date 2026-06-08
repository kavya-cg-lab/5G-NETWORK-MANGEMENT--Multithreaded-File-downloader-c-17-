#include "../include/fileinfo.hpp"
#include <iostream>
#include <curl/curl.h>

size_t FileInfo::discardData(void* buffer, size_t size, size_t nmemb, void* userp) {
    (void)buffer; (void)userp;
    return size * nmemb;
}

// ─────────────────────────────────────────
// FIX: Two completely independent curl handles.
// The original code used curl_easy_reset() which
// wiped the timeout on the second request, causing
// redirect HTML pages (170 bytes) to be returned
// as the "file size" on flaky connections.
// ─────────────────────────────────────────
FileData FileInfo::fetch(const std::string& url) {
    FileData data;

    // ── REQUEST 1: Get real file size ──────────────────────────
    {
        CURL* curl = curl_easy_init();
        if (!curl) {
            std::cerr << "❌ Failed to initialize curl\n";
            return data;
        }

        curl_easy_setopt(curl, CURLOPT_URL,            url.c_str());
        curl_easy_setopt(curl, CURLOPT_NOBODY,          1L);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION,  1L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT,         15L);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT,  10L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,   discardData);

        CURLcode result = curl_easy_perform(curl);

        if (result != CURLE_OK) {
            std::cerr << "❌ Curl error: " << curl_easy_strerror(result) << "\n";
            curl_easy_cleanup(curl);
            return data;
        }

        curl_off_t fileSize = 0;
        curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &fileSize);
        curl_easy_cleanup(curl);   // done — clean up before request 2

        if (fileSize <= 0) {
            std::cerr << "❌ Server did not return a valid Content-Length\n";
            return data;
        }

        data.fileSize = static_cast<long long>(fileSize);
        data.isValid  = true;
    }

    // ── REQUEST 2: Check Range support ─────────────────────────
    // Separate handle, own timeout — no shared state with request 1
    {
        CURL* curl = curl_easy_init();
        if (!curl) {
            data.supportsRange = false;  // safe fallback
        } else {
            curl_easy_setopt(curl, CURLOPT_URL,            url.c_str());
            curl_easy_setopt(curl, CURLOPT_NOBODY,          1L);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION,  1L);
            curl_easy_setopt(curl, CURLOPT_TIMEOUT,         15L);
            curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT,  10L);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,   discardData);

            struct curl_slist* headers = nullptr;
            headers = curl_slist_append(headers, "Range: bytes=0-1");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

            CURLcode result = curl_easy_perform(curl);

            long httpCode = 0;
            if (result == CURLE_OK) {
                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
            }

            data.supportsRange = (httpCode == 206);

            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
        }
    }

    // Extract filename from URL
    size_t lastSlash = url.find_last_of('/');
    data.fileName = (lastSlash != std::string::npos)
                    ? url.substr(lastSlash + 1)
                    : "downloaded_file";

    return data;
}

void FileInfo::print(const FileData& data) {
    std::cout << "\n================================\n";
    std::cout << "       FILE INFORMATION         \n";
    std::cout << "================================\n";

    if (!data.isValid) {
        std::cout << "❌ Could not reach server\n";
        std::cout << "================================\n";
        return;
    }

    std::cout << "✅ File Name  : " << data.fileName << "\n";
    std::cout << "✅ File Size  : " << data.fileSize << " bytes\n";
    std::cout << "            : " << data.fileSize / 1024 << " KB\n";
    std::cout << "            : " << data.fileSize / (1024 * 1024) << " MB\n";
    std::cout << "✅ Range      : "
              << (data.supportsRange ? "Supported ✅" : "Not Supported ❌") << "\n";
    std::cout << "✅ Threading  : "
              << (data.supportsRange ? "Multithreaded ✅" : "Single Thread only") << "\n";
    std::cout << "================================\n";
}


