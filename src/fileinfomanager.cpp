#include "../include/fileinfomanager.hpp"

const FileData& FileInfoManager::fetchFileInfo(const std::string& url) {
    data_  = FileInfo::fetch(url);
    ready_ = true;
    return data_;
}

const FileData& FileInfoManager::getFileInfo() const {
    if (!ready_) throw std::logic_error("Call fetchFileInfo first");
    return data_;
}

void FileInfoManager::printFileInfo(int numThreads) const {
    if (!ready_) throw std::logic_error("Call fetchFileInfo first");
    FileInfo::print(data_, numThreads);
}