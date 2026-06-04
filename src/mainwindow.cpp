#include "../include/mainwindow.h"
#include "../include/downloadworker.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFileDialog>
#include <QGroupBox>
#include <QMessageBox>
#include <QFont>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), isDownloading(false), workerThread(nullptr), downloadWorker(nullptr) {
    setWindowTitle("Multithreaded File Downloader");
    setGeometry(100, 100, 900, 500);
    
    setupUI();
    connectSignals();
}

MainWindow::~MainWindow() {
    if (workerThread) {
        workerThread->quit();
        workerThread->wait();
    }
}

void MainWindow::setupUI() {
    // Central widget
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // Title
    QLabel *titleLabel = new QLabel("Download Manager");
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    mainLayout->addWidget(titleLabel);

    // URL Group
    QGroupBox *urlGroup = new QGroupBox("Download URL", this);
    QVBoxLayout *urlLayout = new QVBoxLayout(urlGroup);
    urlInput = new QLineEdit(this);
    urlInput->setPlaceholderText("Enter download URL (e.g., http://example.com/file.zip)");
    urlLayout->addWidget(urlInput);
    mainLayout->addWidget(urlGroup);

    // Output File Group
    QGroupBox *outputGroup = new QGroupBox("Output File", this);
    QHBoxLayout *outputLayout = new QHBoxLayout(outputGroup);
    outputFileInput = new QLineEdit(this);
    outputFileInput->setPlaceholderText("Select output file path");
    browseButton = new QPushButton("Browse...", this);
    browseButton->setMaximumWidth(100);
    outputLayout->addWidget(outputFileInput);
    outputLayout->addWidget(browseButton);
    mainLayout->addWidget(outputGroup);

    // Settings Group
    QGroupBox *settingsGroup = new QGroupBox("Settings", this);
    QHBoxLayout *settingsLayout = new QHBoxLayout(settingsGroup);
    
    QLabel *threadsLabel = new QLabel("Threads:", this);
    threadsSpinBox = new QSpinBox(this);
    threadsSpinBox->setMinimum(1);
    threadsSpinBox->setMaximum(16);
    threadsSpinBox->setValue(4);
    
    settingsLayout->addWidget(threadsLabel);
    settingsLayout->addWidget(threadsSpinBox);
    settingsLayout->addStretch();
    mainLayout->addWidget(settingsGroup);

    // Progress Group
    QGroupBox *progressGroup = new QGroupBox("Progress", this);
    QVBoxLayout *progressLayout = new QVBoxLayout(progressGroup);
    
    progressBar = new QProgressBar(this);
    progressBar->setValue(0);
    progressLayout->addWidget(progressBar);

    statusLabel = new QLabel("Ready", this);
    progressLayout->addWidget(statusLabel);

    QHBoxLayout *statsLayout = new QHBoxLayout();
    downloadedLabel = new QLabel("Downloaded: 0 MB / 0 MB", this);
    speedLabel = new QLabel("Speed: 0 KB/s", this);
    etaLabel = new QLabel("ETA: --", this);
    
    statsLayout->addWidget(downloadedLabel);
    statsLayout->addWidget(speedLabel);
    statsLayout->addWidget(etaLabel);
    progressLayout->addLayout(statsLayout);
    
    mainLayout->addWidget(progressGroup);

    // Button Group
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    downloadButton = new QPushButton("Download", this);
    cancelButton = new QPushButton("Cancel", this);
    cancelButton->setEnabled(false);
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(downloadButton);
    buttonLayout->addWidget(cancelButton);
    mainLayout->addLayout(buttonLayout);

    mainLayout->addStretch();
}

void MainWindow::connectSignals() {
    connect(browseButton, &QPushButton::clicked, this, &MainWindow::onBrowseClicked);
    connect(downloadButton, &QPushButton::clicked, this, &MainWindow::onDownloadClicked);
    connect(cancelButton, &QPushButton::clicked, this, &MainWindow::onCancelClicked);
}

void MainWindow::onBrowseClicked() {
    QString fileName = QFileDialog::getSaveFileName(this,
        "Save Download As", "",
        "All Files (*)");
    
    if (!fileName.isEmpty()) {
        outputFileInput->setText(fileName);
    }
}

void MainWindow::onDownloadClicked() {
    if (urlInput->text().isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please enter a download URL");
        return;
    }
    
    if (outputFileInput->text().isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please select an output file");
        return;
    }

    // Setup worker thread if not already done
    if (!workerThread) {
        workerThread = new QThread(this);
        downloadWorker = new DownloadWorker();
        downloadWorker->moveToThread(workerThread);

        connect(workerThread, &QThread::finished, downloadWorker, &QObject::deleteLater);
        connect(this, &MainWindow::destroyed, workerThread, &QThread::quit);
        
        connect(downloadWorker, &DownloadWorker::progressUpdated,
                this, &MainWindow::onDownloadProgress);
        connect(downloadWorker, &DownloadWorker::downloadFinished,
                this, &MainWindow::onDownloadFinished);
        connect(downloadWorker, &DownloadWorker::downloadError,
                this, &MainWindow::onDownloadError);

        workerThread->start();
    }

    updateUIState(true);
    statusLabel->setText("Starting download...");
    progressBar->setValue(0);
    
    // Pass 0 threads to use auto-calculator
    int threads = threadsSpinBox->value();
    if (threadsSpinBox->value() == 4) {
        // If user hasn't explicitly changed it from default, use auto-calculator
        threads = 0; // 0 means auto-calculate
    }
    
    QMetaObject::invokeMethod(downloadWorker, "startDownload", Qt::QueuedConnection,
        Q_ARG(QString, urlInput->text()),
        Q_ARG(QString, outputFileInput->text()),
        Q_ARG(int, threads));
}

void MainWindow::onCancelClicked() {
    if (downloadWorker) {
        QMetaObject::invokeMethod(downloadWorker, "cancelDownload", Qt::QueuedConnection);
    }
    updateUIState(false);
    statusLabel->setText("Download cancelled");
}

void MainWindow::onDownloadProgress(long long downloaded, long long total, double speed, int eta) {
    if (total > 0) {
        int progress = static_cast<int>((downloaded * 100) / total);
        progressBar->setValue(progress);
    }

    // Format sizes
    auto formatSize = [](long long bytes) -> QString {
        if (bytes < 1024) return QString::number(bytes) + " B";
        if (bytes < 1024 * 1024) return QString::number(bytes / 1024.0, 'f', 2) + " KB";
        if (bytes < 1024 * 1024 * 1024) return QString::number(bytes / (1024.0 * 1024), 'f', 2) + " MB";
        return QString::number(bytes / (1024.0 * 1024 * 1024), 'f', 2) + " GB";
    };

    downloadedLabel->setText(QString("Downloaded: %1 / %2")
        .arg(formatSize(downloaded))
        .arg(formatSize(total)));

    speedLabel->setText(QString("Speed: %1 KB/s").arg(speed, 0, 'f', 2));

    QString etaText;
    if (eta > 0) {
        int hours = eta / 3600;
        int minutes = (eta % 3600) / 60;
        int seconds = eta % 60;
        if (hours > 0)
            etaText = QString("ETA: %1h %2m %3s").arg(hours).arg(minutes).arg(seconds);
        else if (minutes > 0)
            etaText = QString("ETA: %1m %2s").arg(minutes).arg(seconds);
        else
            etaText = QString("ETA: %1s").arg(seconds);
    } else {
        etaText = "ETA: --";
    }
    etaLabel->setText(etaText);
}

void MainWindow::onDownloadFinished(bool success, const QString &message) {
    updateUIState(false);
    if (success) {
        statusLabel->setText("Download completed successfully!");
        progressBar->setValue(100);
        QMessageBox::information(this, "Success", message);
    } else {
        statusLabel->setText("Download failed!");
        QMessageBox::critical(this, "Error", message);
    }
}

void MainWindow::onDownloadError(const QString &error) {
    updateUIState(false);
    statusLabel->setText("Error occurred!");
    QMessageBox::critical(this, "Download Error", error);
}

void MainWindow::updateUIState(bool downloading) {
    isDownloading = downloading;
    urlInput->setEnabled(!downloading);
    outputFileInput->setEnabled(!downloading);
    browseButton->setEnabled(!downloading);
    threadsSpinBox->setEnabled(!downloading);
    downloadButton->setEnabled(!downloading);
    cancelButton->setEnabled(downloading);
}
