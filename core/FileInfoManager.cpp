#include "FileInfoManager.hpp"
#include <curl/curl.h>
#include <iostream>

long getFileSize(const std::string& url) {

    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cout << "❌ CURL init failed\n";
        return -1;
    }

    curl_off_t fileSize = -1;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);   // HEAD request
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // follow redirects
    curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);

    // ✅ VERY IMPORTANT (debug)
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        std::cout << "❌ CURL Error: " << curl_easy_strerror(res) << "\n";
        curl_easy_cleanup(curl);
        return -1;
    }

    CURLcode info_res = curl_easy_getinfo(
        curl,
        CURLINFO_CONTENT_LENGTH_DOWNLOAD_T,
        &fileSize
    );

    if ((info_res != CURLE_OK) || (fileSize < 0)) {
        std::cout << "❌ Could not fetch file size properly\n";
        fileSize = -1;
    }

    curl_easy_cleanup(curl);
    return (long)fileSize;
}