#include "../include/downloader.hpp"
#include "../include/progress_tracker.hpp"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <thread>
#include <stdexcept>
#include <curl/curl.h>

// ── static definitions ──────────────────────────────────────────────────────
std::vector<std::string> Downloader::threadErrors_;
std::mutex               Downloader::errorMutex_;

// ─────────────────────────────────────────
// Curl write callback
// ─────────────────────────────────────────
struct WriteContext { std::ofstream* file; int threadId; };

size_t Downloader::writeData(void* buf, size_t size, size_t nmemb,
                              void* stream) {
    size_t total = size * nmemb;
    auto*  ctx   = static_cast<WriteContext*>(stream);
    ctx->file->write(static_cast<char*>(buf), total);
    ProgressTracker::updateThread(ctx->threadId,
                                  static_cast<long long>(total));
    return total;
}

// ─────────────────────────────────────────
// downloadChunk
//
// KEY FIXES v2:
// 1. Uses the pre-resolved URL (no redirects needed).
// 2. CURLOPT_FOLLOWLOCATION OFF — prevents curl from silently
//    dropping the Range header on any 3xx it encounters.
// 3. Sends Range header via CURLOPT_RANGE (curl formats it
//    as "bytes=start-end" automatically).
// 4. Accepts BOTH 206 (proper partial content) and 200
//    (server ignores range but sends full content — only
//    valid for single-thread fallback).
// 5. Retries up to MAX_RETRIES on transient network errors.
// 6. Resets the per-thread byte counter before each retry
//    so progress is not double-counted.
// ─────────────────────────────────────────
static constexpr int MAX_RETRIES = 3;

void Downloader::downloadChunk(const std::string& resolvedUrl,
                                const ChunkInfo& chunk) {
    try {
        if (chunk.startByte > chunk.endByte) {
            throw std::runtime_error(
                "Thread " + std::to_string(chunk.threadId)
                + ": invalid range " + std::to_string(chunk.startByte)
                + "-" + std::to_string(chunk.endByte));
        }

        // Format: "startByte-endByte" (curl prepends "bytes=")
        std::string range = std::to_string(chunk.startByte) + "-"
                          + std::to_string(chunk.endByte);

        ProgressTracker::markActive(chunk.threadId);

        int      attempt  = 0;
        CURLcode res      = CURLE_OK;
        long     httpCode = 0;

        while (attempt < MAX_RETRIES) {
            ++attempt;

            // Reset byte counter before each attempt so progress
            // does not double-count retried bytes
            ProgressTracker::resetThread(chunk.threadId);

            std::ofstream file(chunk.tempFile,
                std::ios::binary | std::ios::trunc);
            if (!file) {
                throw std::runtime_error(
                    "Thread " + std::to_string(chunk.threadId)
                    + ": cannot open " + chunk.tempFile);
            }

            CURL* curl = curl_easy_init();
            if (!curl) throw std::runtime_error("curl_easy_init failed");

            WriteContext ctx{ &file, chunk.threadId };

            curl_easy_setopt(curl, CURLOPT_URL,             resolvedUrl.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,   &Downloader::writeData);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA,       &ctx);
            // CURLOPT_RANGE sends "Range: bytes=start-end" automatically
            curl_easy_setopt(curl, CURLOPT_RANGE,           range.c_str());
            curl_easy_setopt(curl, CURLOPT_BUFFERSIZE,      131072L);
            curl_easy_setopt(curl, CURLOPT_TCP_NODELAY,     1L);
            curl_easy_setopt(curl, CURLOPT_USERAGENT,       "Mozilla/5.0");
            // DO NOT follow redirects — URL is already resolved.
            // Following redirects causes curl to silently drop Range.
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION,  0L);
            curl_easy_setopt(curl, CURLOPT_TIMEOUT,         300L);
            curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT,  15L);
            // Disable HTTP/2 multiplexing — use one connection per thread
            curl_easy_setopt(curl, CURLOPT_HTTP_VERSION,    CURL_HTTP_VERSION_1_1);

            res = curl_easy_perform(curl);
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

            file.close();
            curl_easy_cleanup(curl);

            if (res == CURLE_OK && (httpCode == 206 || httpCode == 200))
                break;

            // Non-retryable server errors
            if (httpCode == 416) {
                throw std::runtime_error(
                    "Thread " + std::to_string(chunk.threadId)
                    + ": HTTP 416 Range Not Satisfiable for range bytes="
                    + range + ". The server rejected this range."
                    + " Likely the resolved URL expired or is wrong.");
            }
            if (httpCode == 403 || httpCode == 404) {
                throw std::runtime_error(
                    "Thread " + std::to_string(chunk.threadId)
                    + ": HTTP " + std::to_string(httpCode)
                    + " — access denied or resource not found");
            }

            // Retryable errors
            if (attempt < MAX_RETRIES) {
                ProgressTracker::logLine(
                    "[Thread " + std::to_string(chunk.threadId)
                    + "] Attempt " + std::to_string(attempt)
                    + " failed (" + curl_easy_strerror(res)
                    + "), retrying...");
            }
        }

        if (res != CURLE_OK) {
            throw std::runtime_error(
                "Thread " + std::to_string(chunk.threadId)
                + ": curl error after " + std::to_string(MAX_RETRIES)
                + " attempts: " + curl_easy_strerror(res));
        }
        if (httpCode != 206 && httpCode != 200) {
            throw std::runtime_error(
                "Thread " + std::to_string(chunk.threadId)
                + ": HTTP " + std::to_string(httpCode)
                + " — range request failed (bytes=" + range + ")");
        }

        ProgressTracker::markDone(chunk.threadId, chunk.chunkSize);

    } catch (const std::exception& e) {
        std::lock_guard<std::mutex> lk(errorMutex_);
        threadErrors_.push_back(e.what());
    }
}

// ─────────────────────────────────────────
// downloadAll
// ─────────────────────────────────────────
bool Downloader::downloadAll(const std::string& resolvedUrl,
                              const std::vector<ChunkInfo>& chunks) {
    {
        std::lock_guard<std::mutex> lk(errorMutex_);
        threadErrors_.clear();
    }

    {
        std::string tval = std::to_string(chunks.size());
        std::string tpad(43 - std::min<size_t>(tval.size(), 43), ' ');
        std::cout
            << "\n╔══════════════════════════════════════════════════════╗\n"
            << "║                 STARTING DOWNLOAD                   ║\n"
            << "╠══════════════════════════════════════════════════════╣\n"
            << "║  Threads : " << tval << tpad << "║\n"
            << "╚══════════════════════════════════════════════════════╝\n"
            << std::flush;
    }

    ProgressTracker::logLine("Starting threads...");

    std::vector<std::thread> threads;
    threads.reserve(chunks.size());
    for (const auto& chunk : chunks)
        threads.emplace_back(downloadChunk, resolvedUrl, chunk);
    for (auto& t : threads)
        t.join();

    std::lock_guard<std::mutex> lk(errorMutex_);
    if (!threadErrors_.empty()) {
        std::cerr << "\n[Download errors]\n";
        for (const auto& e : threadErrors_)
            std::cerr << "  " << e << "\n";
        return false;
    }
    return true;
}