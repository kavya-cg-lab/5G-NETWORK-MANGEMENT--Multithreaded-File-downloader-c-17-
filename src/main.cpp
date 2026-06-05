#include <iostream>
#include <string>

#include "../include/validator.hpp"
#include "../include/fileinfo.hpp"
#include "../include/fileinfomanager.hpp"
#include "../include/chunkcalculator.hpp"
#include "../include/chunkmanager.hpp"
#include "../include/downloader.hpp"
#include "../include/filemerger.hpp"

int main() {

    // ─────────────────────────────────────
    // User Input
    // ─────────────────────────────────────
    std::string url   = "http://speedtest.tele2.net/10GB.zip";
    int numThreads = 4;
    std::string outputName = "myfile.zip";

    std::cout << "\n=============================\n";
    std::cout << " MULTITHREADED FILE DOWNLOADER\n";
    std::cout << "=============================\n";

    // ─────────────────────────────────────
    // Step 1 — Validate Input
    // ─────────────────────────────────────
    std::cout << "\n[STEP 1] Validating input...\n";

    if (!Validator::validateAll(url, numThreads, outputName)) {
        std::cerr << "\n❌ Validation failed. Exiting.\n";
        return 1;
    }

    std::cout << "\n✅ Step 1 Complete — Input Valid\n";

    // ─────────────────────────────────────
    // Step 2 — Fetch File Info
    // ─────────────────────────────────────
    std::cout << "\n[STEP 2] Fetching file info...\n";

    //FileData fileData = FileInfo::fetch(url);
    //FileInfo::print(fileData);
    
    FileInfoManager fileManager;

    FileData fileData = fileManager.fetchFileInfo(url);
    fileManager.printFileInfo(fileData);

    if (!fileData.isValid) {
        std::cerr << "\n❌ Could not fetch file info. Exiting.\n";
        return 1;
    }

    // If no range support — limit to 1 thread
    if (!fileData.supportsRange) {
        std::cout << "⚠️  Range not supported.\n";
        std::cout << "   Switching to single thread.\n";
        numThreads = 1;
    }

    std::cout << "\n✅ Step 2 Complete — File Info Ready\n";

    // ─────────────────────────────────────
    // Step 3 — Calculate Chunks
    // ─────────────────────────────────────
    std::cout << "\n[STEP 3] Calculating chunks...\n";

    ChunkManager manager;

    auto chunks = manager.divideIntoChunks(fileData.fileSize, numThreads);
    manager.printChunks(chunks);

    std::cout << "\n✅ Step 3 Complete — Chunks Ready\n";
    std::cout << "\n Next Step: Download chunks using threads\n\n";
    // ─────────────────────────────────────
// Step 4 — Download Chunks
// ─────────────────────────────────────
    std::cout << "\n[STEP 4] Downloading chunks...\n";

    Downloader::downloadAll(url, chunks);

    std::cout << "\n✅ Step 4 Complete — Download Finished\n";
    std::cout << "\n Next Step: Merge chunk files\n\n";

    // ─────────────────────────────────────
// Step 5 — Merge Chunks
// ─────────────────────────────────────
    std::cout << "\n[STEP 5] Merging chunks...\n";

    if (!FileMerger::merge(chunks, outputName)) {
       std::cerr << "❌ Merge failed\n";
       return 1;
    }

    std::cout << "\n✅ Step 5 Complete — File Ready\n";

    return 0;
}