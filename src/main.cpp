#include <iostream>
#include <iomanip>
#include <string>
#include <stdexcept>
#include <fstream>
#include <chrono>

#include "fileinfomanager.hpp"
#include "chunkmanager.hpp"
#include "downloader.hpp"
#include "filemerger.hpp"
#include "progress_tracker.hpp"
#include "validator.hpp"

enum ExitCode : int {
    OK                   = 0,
    ERR_USAGE            = 1,
    ERR_INVALID_URL      = 2,
    ERR_INVALID_THREADS  = 3,
    ERR_INVALID_ARG      = 4,
    ERR_SERVER           = 5,
    ERR_INVALID_FILESIZE = 6,
    ERR_NO_RANGE         = 7,
    ERR_CHUNK            = 8,
    ERR_DOWNLOAD         = 9,
    ERR_MERGE            = 10,
    ERR_UNKNOWN          = 99,
};

static void printError(const std::string& stage,
                       const std::string& msg, int code) {
    std::cerr << "\n[ERROR] " << stage << ": " << msg
              << "  (exit " << code << ")\n";
}
static void printUsage(const char* prog) {
    std::cerr << "Usage:   " << prog << " <url> <num_threads>\n"
              << "Example: " << prog
              << " http://speedtest.tele2.net/100MB.zip 4\n"
              << "Threads: 1 – 64\n";
}
static long long localFileSize(const std::string& path) {
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    return f ? static_cast<long long>(f.tellg()) : -1LL;
}

int main(int argc, char* argv[]) {
    try {

        // ── Step 0: args ────────────────────────────────────────────
        if (argc < 3) { printUsage(argv[0]); return ERR_USAGE; }

        const std::string url = argv[1];
        int numThreads = 0;
        try {
            numThreads = std::stoi(argv[2]);
        } catch (const std::invalid_argument&) {
            printError("Argument",
                std::string("'") + argv[2] + "' is not an integer",
                ERR_INVALID_ARG);
            return ERR_INVALID_ARG;
        } catch (const std::out_of_range&) {
            printError("Argument",
                std::string("'") + argv[2] + "' is out of range",
                ERR_INVALID_ARG);
            return ERR_INVALID_ARG;
        }

        // ── Step 1: validate inputs ──────────────────────────────────
        if (!Validator::isValidUrl(url)) {
            Validator::printUrlError(url);
            return ERR_INVALID_URL;
        }
        if (!Validator::isValidThreadCount(numThreads)) {
            Validator::printThreadCountError(numThreads);
            return ERR_INVALID_THREADS;
        }

        // ── Step 2: fetch metadata ───────────────────────────────────
        // FileInfo::fetch follows redirects and stores the final URL
        // in FileData::resolvedUrl — Downloader uses that directly,
        // so no thread ever sends a Range request to a redirect URL.
        FileInfoManager fim;
        const FileData& info = fim.fetchFileInfo(url);

        int threads = numThreads;
        if (!info.supportsRange) {
            std::cout
                << "[INFO] Server does not support range requests."
                   " Falling back to 1 thread.\n";
            threads = 1;
        }

        fim.printFileInfo(threads);   // shows File Name, Size, Range, Threads

        if (!info.isValid) {
            printError("Server", "Could not reach server or file empty",
                       ERR_SERVER);
            return ERR_SERVER;
        }
        if (!Validator::isValidFileSize(info.fileSize)) {
            printError("Server",
                "Invalid file size: " + std::to_string(info.fileSize),
                ERR_INVALID_FILESIZE);
            return ERR_INVALID_FILESIZE;
        }

        // ── Step 3: chunk breakdown ──────────────────────────────────
        ChunkManager cm;
        try {
            cm.divideIntoChunks(info.fileSize, threads);
        } catch (const std::exception& e) {
            printError("Chunk", e.what(), ERR_CHUNK);
            return ERR_CHUNK;
        }
        cm.printChunks();

        // ── Step 4: init tracker and download ───────────────────────
        ProgressTracker::init(info.fileSize, threads);

        auto dlStart = std::chrono::steady_clock::now();

        // Pass the pre-resolved URL — never the original redirect URL
        if (!Downloader::downloadAll(info.resolvedUrl, cm.getChunks())) {
            printError("Download", "One or more threads failed",
                       ERR_DOWNLOAD);
            return ERR_DOWNLOAD;
        }

        double dlSec = std::chrono::duration<double>(
            std::chrono::steady_clock::now() - dlStart).count();

        // ── Step 5: merge ────────────────────────────────────────────
        if (!FileMerger::merge(cm.getChunks(), info.fileName)) {
            printError("Merge",
                "Failed to merge into '" + info.fileName + "'",
                ERR_MERGE);
            return ERR_MERGE;
        }

        // ── Step 6: verify and summarise ────────────────────────────
        long long mergedSize = localFileSize(info.fileName);
        bool      sizeOk     = (mergedSize == info.fileSize);

        ProgressTracker::finish(mergedSize);

        std::cout << "\n";
        std::cout << "  File     : " << info.fileName << "\n";
        std::cout << "  Merge    : "
                  << (sizeOk ? "OK ✓" : "FAILED ✗") << "\n";
        std::cout << "  Expected : " << info.fileSize << " bytes\n";
        std::cout << "  Got      : " << mergedSize    << " bytes\n";
        std::cout << "  Time     : " << std::fixed
                  << std::setprecision(2) << dlSec << " s\n";

        return sizeOk ? OK : ERR_MERGE;

    } catch (const std::exception& e) {
        printError("Unexpected", e.what(), ERR_UNKNOWN);
        return ERR_UNKNOWN;
    } catch (...) {
        printError("Unexpected", "unknown exception", ERR_UNKNOWN);
        return ERR_UNKNOWN;
    }
}