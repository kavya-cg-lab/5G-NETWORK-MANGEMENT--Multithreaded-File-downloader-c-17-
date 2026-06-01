# 5G Network Management System
## Multithreaded File Downloader (C++17)

---

## 📌 Overview

This project implements a **Multithreaded File Downloader** using **C++17**, inspired by real-world **5G Network Management Systems (5G-NMS)**.

It simulates the **File Transfer Engine** responsible for delivering firmware updates from a central server to multiple network nodes.

The system:
- Splits a file into chunks
- Downloads chunks in parallel using threads
- Tracks real-time progress
- Merges chunks into a final file
- Ensures data integrity

---

## System Architecture

Below is the UML design representing system structure and class interactions:

<p align="center">
  assets/architecture.png
</p>

---

## 🔄 Workflow

1. User provides input (`URL`, `threads`, `output`)
2. `MainController` initializes system
3. `FileInfoManager` retrieves file size and range support
4. `ChunkManager` divides file into chunks
5. `DownloaderFactory` selects strategy
6. `ThreadManager` creates threads
7. Each `Downloader` downloads chunk
8. `TempFileHandler` writes chunks
9. `ProgressTracker` updates progress
10. `FileMerger` merges final output

---

## 📁 Project Structure
