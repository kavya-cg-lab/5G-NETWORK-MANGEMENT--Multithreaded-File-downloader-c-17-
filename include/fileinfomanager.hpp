#pragma once
#include <string>
#include <stdexcept>
#include "fileinfo.hpp"

// ─────────────────────────────────────────
// Owns and caches FileData for a session
// ─────────────────────────────────────────
class FileInfoManager {
public:
    const FileData& fetchFileInfo(const std::string& url);
    const FileData& getFileInfo() const;
    void printFileInfo(int numThreads = 1) const;
    bool hasInfo() const { return ready_; }
    void reset() { data_ = FileData{}; ready_ = false; }

private:
    FileData data_;
    bool     ready_ = false;
};