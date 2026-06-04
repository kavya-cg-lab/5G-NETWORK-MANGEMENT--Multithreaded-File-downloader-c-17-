#pragma once

#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QProgressBar>
#include <QLabel>
#include <QSpinBox>
#include <QThread>
#include <memory>

class DownloadWorker;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onBrowseClicked();
    void onDownloadClicked();
    void onCancelClicked();
    void onDownloadProgress(long long downloaded, long long total, double speed, int eta);
    void onDownloadFinished(bool success, const QString &message);
    void onDownloadError(const QString &error);

private:
    void setupUI();
    void connectSignals();
    void updateUIState(bool downloading);

    // UI Components
    QLineEdit *urlInput;
    QLineEdit *outputFileInput;
    QSpinBox *threadsSpinBox;
    QPushButton *browseButton;
    QPushButton *downloadButton;
    QPushButton *cancelButton;
    QProgressBar *progressBar;
    QLabel *statusLabel;
    QLabel *speedLabel;
    QLabel *etaLabel;
    QLabel *downloadedLabel;

    // Worker and thread
    QThread *workerThread;
    DownloadWorker *downloadWorker;
    bool isDownloading;
};
