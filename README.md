# 5G Network Management System - Multithreaded File Downloader

A high-performance **C++17** multithreaded file downloader inspired by 5G Network Management Systems, designed for efficient parallel file transfers with real-time progress tracking and data integrity validation.

## 📋 Table of Contents

- [Overview](#-overview)
- [Features](#-features)
- [System Architecture](#system-architecture)
- [Prerequisites](#-prerequisites)
- [Installation & Setup](#-installation--setup)
- [Building](#-building)
- [Usage](#-usage)
- [Project Structure](#-project-structure)
- [Key Components](#-key-components)
- [Technical Details](#-technical-details)
- [Workflow](#-workflow)
- [Contributing](#-contributing)

---

## 📌 Overview

This project implements a **Multithreaded File Downloader** for **5G Network Management**, simulating the File Transfer Engine responsible for delivering firmware updates and large files from a central server to multiple network nodes.

The application features a **Qt5 GUI** interface combined with a robust backend that:
- Splits files into manageable chunks
- Downloads chunks in parallel using worker threads (1-16 threads configurable)
- Provides real-time progress tracking and metadata
- Merges downloaded chunks into the final file
- Validates data integrity throughout the process
- Logs all operations with detailed error handling

**Technology Stack:** C++17, Qt5, Multi-threading, Network I/O

---

## ⚡ Features

- **Intelligent Chunk Division** - Automatically calculates optimal chunk sizes based on file size and thread count
- **Parallel Downloads** - Configurable thread pool (1-16 threads) for simultaneous chunk downloads
- **Real-time Progress Tracking** - Displays download speed, ETA, percentage completed, and bytes transferred
- **Range Request Support** - Uses HTTP Range header for partial file downloads
- **Smart Thread Calculator** - Automatically suggests optimal thread count based on file size
- **Data Validation** - Input validation for URLs, thread counts, and output filenames
- **Error Handling & Logging** - Comprehensive logging system with detailed error messages
- **GUI Interface** - User-friendly Qt5-based interface for easy file downloads
- **Cross-Platform** - Runs on Linux and other Unix-like systems

---

## System Architecture

```
┌─────────────────────────────────────────────────────────┐
│                    MainWindow (Qt GUI)                  │
│              User Interface & Input Handling             │
└──────────────────┬──────────────────────────────────────┘
                   │
┌──────────────────▼──────────────────────────────────────┐
│              MainController (Orchestrator)              │
│         Coordinates all system components               │
└──┬────────────────────────────────────────────┬─────────┘
   │                                            │
   ├─→ Validator (Input Validation)            │
   ├─→ FileInfoManager (Metadata Retrieval)    │
   ├─→ ChunkManager (File Division)            │
   ├─→ DownloadSystem (Multithreaded Engine)   │
   ├─→ ProgressTracker (Real-time Stats)       │
   └─→ FileMerger (Final Assembly)             │
```

---

## 🔧 Prerequisites

- **Operating System:** Linux (Ubuntu 18.04+, CentOS 7+, or compatible)
- **C++ Compiler:** GCC 7+ or Clang 5+ with C++17 support
- **Qt Framework:** Qt 5.9+ (Core, Gui, Network modules)
- **Build Tools:** 
  - CMake 3.10+ OR
  - QMake (included with Qt)
  - Make

**Installation on Ubuntu:**
```bash
sudo apt-get update
sudo apt-get install build-essential qt5-qmake qt5-default libqt5core5a libqt5gui5 libqt5network5
```

**Installation on CentOS/RHEL:**
```bash
sudo yum groupinstall "Development Tools"
sudo yum install qt5-qtbase-devel qt5-qtbase
```

---

## 📦 Installation & Setup

1. **Clone or Extract Repository**
```bash
cd /path/to/5G-NETWORK-MANGEMENT--Multithreaded-File-downloader-c-17-
```

2. **Verify Project Structure**
```bash
ls -la
```

Expected directories: `src/`, `include/`, `ui/`, `assets/`

---

## 🔨 Building

### Option 1: Using QMake (Recommended)

```bash
# Clean previous builds
make clean

# Build the project
qmake downloader.pro
make -j$(nproc)
```

### Option 2: Using Makefile (if available)

```bash
make
```

### Output
- **Executable:** `./downloader` (in project root)

---

## 🚀 Usage

### Running the Application

```bash
./downloader
```

### GUI Usage

1. **Enter Download URL**
   - Example: `http://example.com/largefile.zip`
   - Must be a valid HTTP/HTTPS URL

2. **Select Thread Count**
   - Range: 1-16 threads
   - Auto-suggestion based on file size
   - Higher threads = faster download (with diminishing returns)

3. **Specify Output Filename**
   - Example: `downloaded_file.zip`
   - Must be a valid filename (no special characters)

4. **Click Download**
   - Real-time progress updates
   - Download speed displayed
   - Estimated time remaining (ETA)
   - Bytes transferred counter

---

## 📁 Project Structure

```
.
├── src/                           # Source files (.cpp)
│   ├── main.cpp                   # Application entry point
│   ├── mainwindow.cpp             # Qt main window implementation
│   ├── maincontroller.cpp         # System orchestrator
│   ├── downloader*.cpp            # Download engine components
│   ├── fileinfo*.cpp              # File metadata handling
│   ├── chunkmanager.cpp           # Chunk division logic
│   ├── filemerger.cpp             # Chunk merging
│   ├── progresstracker.cpp        # Progress tracking
│   ├── validator.cpp              # Input validation
│   ├── logger.cpp                 # Logging system
│   └── ui.cpp                     # UI components
│
├── include/                       # Header files (.hpp/.h)
│   ├── mainwindow.h               # Qt main window class
│   ├── downloadworker.h           # Worker thread class
│   ├── maincontroller.hpp         # Controller orchestration
│   ├── downloadsystem.hpp         # Download engine
│   ├── chunkmanager.hpp           # Chunk management
│   ├── fileinfo*.hpp              # File metadata structures
│   ├── filemerger.hpp             # File merging logic
│   ├── progresstracker.hpp        # Progress information
│   ├── threadautocalculator.hpp   # Thread optimization
│   ├── validator.hpp              # Input validators
│   └── logger.hpp                 # Logging utilities
│
├── ui/                            # Qt Designer files
│   └── mainwindow.ui              # Main window UI definition
│
├── assets/                        # Resources and documentation
│   └── architecture.png           # System architecture diagram
│
├── downloader.pro                 # Qt project configuration
├── Makefile                       # Build configuration
└── README.md                      # This file
```

---

## 🏗️ Key Components

| Component | Purpose |
|-----------|---------|
| **MainWindow** | Qt GUI interface for user interaction |
| **MainController** | Central orchestrator managing all subsystems |
| **FileInfoManager** | Retrieves file metadata from server (size, ranges) |
| **ChunkManager** | Divides file into optimal chunks |
| **DownloadSystem** | Manages multithreaded download operations |
| **DownloadWorker** | Qt worker thread for parallel downloads |
| **ProgressTracker** | Real-time progress statistics and updates |
| **FileMerger** | Assembles downloaded chunks into final file |
| **Validator** | Validates URLs, thread counts, and filenames |
| **Logger** | Centralized logging with error tracking |

---

## 🔬 Technical Details

### Threading Model

- **Worker/Producer Pattern**: Each download chunk is processed by a dedicated worker thread
- **Qt Signal/Slot Mechanism**: Thread-safe communication between workers and main thread
- **Configurable Pool**: Linear scaling up to 16 concurrent threads
- **Thread Safety**: Mutex-protected shared resources (temp files, progress tracking)

### Chunk Calculation

The system intelligently calculates chunk sizes:
```
Optimal Chunk Size = File Size / (Thread Count × Efficiency Factor)
Adjusted for: Network bandwidth, File type, Available memory
```

### Progress Tracking

Real-time metrics provided:
- **Download Speed** - Current bytes/second
- **ETA** - Estimated time to completion
- **Percentage** - Overall progress (0-100%)
- **Bytes Transferred** - Total downloaded vs. total size

### Error Handling

Robust error handling for:
- Invalid URLs or network errors
- Server-side errors (4xx, 5xx responses)
- Incomplete chunk downloads
- File system errors (permissions, space)
- Connection timeouts and retries

---

## 🔄 Workflow

1. **User Input** → Provides URL, thread count, output filename
2. **Validation** → Validator checks inputs (URL format, thread range 1-16, filename validity)
3. **Metadata Fetch** → FileInfoManager queries server for file size and range support
4. **Chunk Planning** → ChunkManager calculates optimal chunk boundaries
5. **Thread Pool Creation** → DownloadSystem spawns worker threads
6. **Parallel Download** → Each thread downloads assigned chunk via HTTP Range requests
7. **Progress Updates** → ProgressTracker monitors and broadcasts real-time stats
8. **Temp File Write** → Downloaded chunks written to temporary files
9. **Integrity Check** → Optional verification of downloaded data
10. **Final Merge** → FileMerger combines chunks into final output file
11. **Cleanup** → Temporary files removed, resources deallocated
12. **Completion** → User notified with final status

---

## 🧪 Testing

Test the downloader with these public URLs:
- `http://www.ovh.net/files/1Mb.dat` (1 MB)
- `http://speedtest.ftp.otenet.gr/files/test100k.db` (100 KB)
- `http://www.ovh.net/files/10Mb.dat` (10 MB)

---

## 📝 Contributing

Contributions are welcome! Areas for enhancement:
- HTTPS/SSL certificate validation
- Resume interrupted downloads
- Bandwidth throttling controls
- Additional file checksum algorithms
- Performance optimization for very large files (>1GB)

---

## 📄 License

This project is part of the 5G Network Management System academic initiative.

---

## 📞 Support & Documentation

For detailed component documentation, refer to header files in `include/` directory. Each class includes inline documentation describing methods, parameters, and return values.

**Common Issues:**
- Qt libraries not found: `sudo apt-get install libqt5core5a libqt5gui5 libqt5network5`
- Build fails: Ensure C++17 standard is supported: `g++ --version`
- Permission denied: `chmod +x downloader` and run with `./downloader`

---

**Last Updated:** June 2026  
**Version:** 1.0.0  
**Language:** C++17  
**Platform:** Linux
