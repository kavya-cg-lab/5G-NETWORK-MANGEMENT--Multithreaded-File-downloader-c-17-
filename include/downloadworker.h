#pragma once

#include <QObject>
#include <QString>
#include <memory>
#include <functional>

class MainController;

class DownloadWorker : public QObject {
    Q_OBJECT

public:
    DownloadWorker();
    ~DownloadWorker();

public slots:
    void startDownload(const QString &url, const QString &outputFile, int threads);
    void cancelDownload();

signals:
    void progressUpdated(long long downloaded, long long total, double speed, int eta);
    void downloadFinished(bool success, const QString &message);
    void downloadError(const QString &error);

private:
    std::unique_ptr<MainController> controller;
    bool shouldCancel;
    
    // Callback for progress updates
    void onProgressUpdate(long long downloaded, long long total, double speedKBps, int etaSeconds);
};

