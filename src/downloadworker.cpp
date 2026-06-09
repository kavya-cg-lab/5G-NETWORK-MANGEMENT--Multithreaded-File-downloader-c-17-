#include "../include/downloadworker.h"
#include "../include/maincontroller.hpp"
#include "../include/progresstracker.hpp"
#include <QCoreApplication>

DownloadWorker::DownloadWorker()
    : controller(nullptr), shouldCancel(false) {
}

DownloadWorker::~DownloadWorker() {
}

void DownloadWorker::startDownload(const QString &url, const QString &outputFile, int threads) {
    try {
        shouldCancel = false;
        // reset global cancel token when starting a new download
        setGlobalCancel(false);
        
        std::string urlStr = url.toStdString();
        std::string outputStr = outputFile.toStdString();

        // If threads is 0, use auto-calculation (default to 8, will be reduced based on file size)
        if (threads <= 0) {
            threads = 8; // Will be auto-adjusted by ThreadAutoCalculator
        }

        // Create a new controller with the provided parameters
        controller = std::make_unique<MainController>(urlStr, threads, outputStr);
        controller->setProgressCallback([this](long long downloaded,
                                               long long total,
                                               double speedKBps,
                                               int etaSeconds) {
            emit progressUpdated(downloaded, total, speedKBps, etaSeconds);
        });
        controller->setThreadCountCallback([this](int count){
            emit threadCountDetermined(count);
        });

        // Initialize the system
        if (!controller->initializeSystem()) {
            emit downloadFinished(false, "Failed to initialize download system. Please check the URL and try again.");
            return;
        }

        // Start the actual download
        // Note: The progress callback is automatically used by ProgressTracker
        if (!controller->startDownload()) {
            // If cancelled by user, emit a clear cancelled message
            if (shouldCancel || isGlobalCancelled()) {
                emit downloadFinished(false, "Download cancelled by user.");
            } else {
                emit downloadFinished(false, "Download failed. Please check the URL and try again.");
            }
            return;
        }

        emit downloadFinished(true, "File downloaded successfully!");
    } catch (const std::exception &e) {
        emit downloadError(QString::fromStdString(e.what()));
        emit downloadFinished(false, QString::fromStdString(e.what()));
    }
}

void DownloadWorker::cancelDownload() {
    shouldCancel = true;
    // set global cancel token so download threads and libcurl will abort
    setGlobalCancel(true);
}

void DownloadWorker::onProgressUpdate(long long downloaded, long long total, double speedKBps, int etaSeconds) {
    emit progressUpdated(downloaded, total, speedKBps, etaSeconds);
    QCoreApplication::processEvents(); // Keep UI responsive
}
