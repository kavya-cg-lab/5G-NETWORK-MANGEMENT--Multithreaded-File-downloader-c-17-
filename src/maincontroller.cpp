// #include "../include/maincontroller.hpp"
// #include "../include/ui.hpp"
// #include <curl/curl.h>
// #include <iostream>

// MainController::MainController(std::string url,
//                                int threadCount,
//                                std::string outputName)
//     : url(std::move(url)),
//       threadCount(threadCount),
//       outputName(std::move(outputName)) {
// }

// MainController::~MainController() {
//     curl_global_cleanup();
// }

// bool MainController::initializeSystem() {
//     UIManager::showStepStart("SYSTEM INIT");

//     CURLcode result = curl_global_init(CURL_GLOBAL_DEFAULT);
//     if (result != CURLE_OK) {
//         UIManager::showFinalResult(false,
//                                    std::string("curl_global_init failed: ")
//                                    + curl_easy_strerror(result));
//         return false;
//     }

//     UIManager::showFinalResult(true, "System initialized.");
//     return true;
// }

// bool MainController::startDownload() {
//     FileData fileData;
//     std::vector<ChunkInfo> chunks;

//     if (!retryStep("STEP 1", [this]() { return validateInputs(); })) {
//         return false;
//     }

//     if (!retryStep("STEP 2", [&]() { return fetchMetadata(fileData); })) {
//         return false;
//     }

//     if (!retryStep("STEP 3", [&]() { return prepareChunks(fileData, chunks); })) {
//         return false;
//     }

//     if (!retryStep("STEP 4", [&]() { return performDownload(fileData, chunks); })) {
//         return false;
//     }

//     std::cout << "\n✅ Download completed successfully!\n";
//     return true;
// }

// bool MainController::validateInputs() {
//     UIManager::showStepStart("STEP 1 - Validating inputs");
//     if (!Validator::validateAll(url, threadCount, outputName)) {
//         UIManager::showFinalResult(false, "Validation failed.");
//         return false;
//     }

//     UIManager::showFinalResult(true, "Step 1 Complete — Input Valid");
//     return true;
// }

// bool MainController::fetchMetadata(FileData& data) {
//     UIManager::showStepStart("STEP 2 - Fetching file metadata");
//     data = fileInfoManager.fetchFileInfo(url);
//     fileInfoManager.printFileInfo(data);

//     if (!data.isValid) {
//         UIManager::showFinalResult(false, "Could not fetch file info.");
//         return false;
//     }

//     if (!data.supportsRange) {
//         UIManager::showStepStatus("⚠️  Range not supported. Switching to single thread.");
//         threadCount = 1;
//     }

//     UIManager::showFinalResult(true, "Step 2 Complete — File Info Ready");
//     return true;
// }

// bool MainController::prepareChunks(const FileData& data,
//                                    std::vector<ChunkInfo>& chunks) {
//     UIManager::showStepStart("STEP 3 - Preparing chunks");
//     chunks = chunkManager.divideIntoChunks(data.fileSize, threadCount);
//     chunkManager.printChunks(chunks);

//     if (chunks.empty()) {
//         UIManager::showFinalResult(false, "Failed to create download chunks.");
//         return false;
//     }

//     if (threadCount == 1 && !chunks.empty()) {
//         chunks[0].tempFile = outputName;
//     }

//     UIManager::showFinalResult(true, "Step 3 Complete — Chunks Ready");
//     return true;
// }

// bool MainController::performDownload(const FileData& data,
//                                      std::vector<ChunkInfo>& chunks) {
//     UIManager::showStepStart("STEP 4 - Downloading chunks");

//     auto downloader = DownloaderFactory::createDownloader(threadCount);
//     if (!downloader) {
//         UIManager::showFinalResult(false, "Failed to select download strategy.");
//         return false;
//     }

//     bool success = downloader->download(url, chunks, outputName);
//     if (!success && threadCount > 1) {
//         UIManager::showStepStatus("⚠️  Multithreaded download failed, retrying with a single thread...");
//         threadCount = 1;
//         chunks = chunkManager.divideIntoChunks(data.fileSize, threadCount);
//         if (!chunks.empty()) {
//             chunks[0].tempFile = outputName;
//         }

//         downloader = DownloaderFactory::createDownloader(threadCount);
//         if (!downloader) {
//             UIManager::showFinalResult(false, "Failed to select fallback download strategy.");
//             return false;
//         }

//         success = downloader->download(url, chunks, outputName);
//     }

//     if (!success) {
//         UIManager::showFinalResult(false, "Download failed.");
//     }
//     return success;
// }

// template <typename Fn>
// bool MainController::retryStep(const std::string& stepName, Fn&& step) {
//     for (int attempt = 1; attempt <= maxRetryCount; ++attempt) {
//         if (attempt > 1) {
//             UIManager::showRetry(attempt, maxRetryCount);
//         }

//         if (step()) {
//             return true;
//         }

//         std::cerr << "⚠️  " << stepName << " attempt "
//                   << attempt << " failed." << "\n";
//     }

//     std::cerr << "❌ " << stepName << " failed after "
//               << maxRetryCount << " attempts.\n";
//     return false;
// }



#include "../include/maincontroller.hpp"
#include "../include/threadautocalculator.hpp"
#include "../include/ui.hpp"
#include <curl/curl.h>
#include <iostream>

MainController::MainController(std::string url, int threadCount, std::string outputName)
    : url(std::move(url)),
      threadCount(threadCount),
      outputName(std::move(outputName)),
      progressCallback(nullptr) {}

MainController::~MainController() {
    curl_global_cleanup();
}

void MainController::setProgressCallback(ProgressTracker::ProgressCallback callback) {
    progressCallback = std::move(callback);
}

void MainController::setThreadCountCallback(std::function<void(int)> callback) {
    threadCountCallback = std::move(callback);
}

bool MainController::initializeSystem() {
    UIManager::showStepStart("SYSTEM INIT");
    CURLcode result = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (result != CURLE_OK) {
        UIManager::showFinalResult(false,
            std::string("curl_global_init failed: ") + curl_easy_strerror(result));
        return false;
    }
    UIManager::showFinalResult(true, "System initialized.");
    return true;
}

bool MainController::startDownload() {
    FileData fileData;
    std::vector<ChunkInfo> chunks;

    if (!retryStep("STEP 1", [this]() { return validateInputs();                })) return false;
    if (!retryStep("STEP 2", [&]()   { return fetchMetadata(fileData);           })) return false;
    if (!retryStep("STEP 3", [&]()   { return prepareChunks(fileData, chunks);   })) return false;
    if (!retryStep("STEP 4", [&]()   { return performDownload(fileData, chunks); })) return false;

    std::cout << "\n✅ Download completed successfully!\n";
    return true;
}

bool MainController::validateInputs() {
    UIManager::showStepStart("STEP 1 - Validating inputs");
    if (!Validator::validateAll(url, threadCount, outputName)) {
        UIManager::showFinalResult(false, "Validation failed.");
        return false;
    }
    UIManager::showFinalResult(true, "Step 1 Complete — Input Valid");
    return true;
}

bool MainController::fetchMetadata(FileData& data) {
    UIManager::showStepStart("STEP 2 - Fetching file metadata");
    data = fileInfoManager.fetchFileInfo(url);
    fileInfoManager.printFileInfo(data);

    if (!data.isValid) {
        UIManager::showFinalResult(false, "Could not fetch file info.");
        return false;
    }

    if (!data.supportsRange) {
        UIManager::showStepStatus("⚠️  Range not supported. Switching to single thread.");
        threadCount = 1;
    } else {
        // Auto-calculate best thread count; user value is the cap
        int autoThreads = ThreadAutoCalculator::calculate(data.fileSize, threadCount);
        if (autoThreads != threadCount) {
            std::cout << "  ℹ️  Auto thread count: " << threadCount
                      << " → " << autoThreads
                      << " (file: " << data.fileSize / (1024*1024) << " MB)\n";
            threadCount = autoThreads;
        } else {
            std::cout << "  ℹ️  Using " << threadCount << " threads"
                      << " for " << data.fileSize / (1024*1024) << " MB file\n";
        }
        if (threadCountCallback) threadCountCallback(threadCount);
    }

    UIManager::showFinalResult(true, "Step 2 Complete — File Info Ready");
    return true;
}

bool MainController::prepareChunks(const FileData& data, std::vector<ChunkInfo>& chunks) {
    UIManager::showStepStart("STEP 3 - Preparing chunks");
    chunks = chunkManager.divideIntoChunks(data.fileSize, threadCount);
    chunkManager.printChunks(chunks);

    if (chunks.empty()) {
        UIManager::showFinalResult(false, "Failed to create download chunks.");
        return false;
    }

    // Single-thread: write directly to outputName — no merge step needed
    if (threadCount == 1 && !chunks.empty()) {
        chunks[0].tempFile = outputName;
    }

    UIManager::showFinalResult(true, "Step 3 Complete — Chunks Ready");
    return true;
}

bool MainController::performDownload(const FileData& data, std::vector<ChunkInfo>& chunks) {
    UIManager::showStepStart("STEP 4 - Downloading chunks");

    auto downloader = DownloaderFactory::createDownloader(threadCount);
    if (!downloader) {
        UIManager::showFinalResult(false, "Failed to select download strategy.");
        return false;
    }

    bool success = downloader->download(url, chunks, outputName, progressCallback);

    // Fallback: multi-thread failed → retry single-thread
    if (!success && threadCount > 1) {
        UIManager::showStepStatus(
            "⚠️  Multithreaded download failed, retrying with single thread...");
        threadCount = 1;
        chunks = chunkManager.divideIntoChunks(data.fileSize, threadCount);
        if (!chunks.empty()) chunks[0].tempFile = outputName;

        downloader = DownloaderFactory::createDownloader(threadCount);
        if (!downloader) {
            UIManager::showFinalResult(false, "Failed to select fallback strategy.");
            return false;
        }
        success = downloader->download(url, chunks, outputName, progressCallback);
    }

    if (!success) UIManager::showFinalResult(false, "Download failed.");
    return success;
}

template <typename Fn>
bool MainController::retryStep(const std::string& stepName, Fn&& step) {
    for (int attempt = 1; attempt <= maxRetryCount; ++attempt) {
        if (attempt > 1) UIManager::showRetry(attempt, maxRetryCount);
        if (step()) return true;
        std::cerr << "⚠️  " << stepName << " attempt " << attempt << " failed.\n";
    }
    std::cerr << "❌ " << stepName << " failed after " << maxRetryCount << " attempts.\n";
    return false;
}