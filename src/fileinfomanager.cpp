#include "../include/fileinfomanager.hpp"

FileData FileInfoManager::fetchFileInfo(const std::string& url) const {
    return FileInfo::fetch(url);
}

void FileInfoManager::printFileInfo(const FileData& data) const {
    FileInfo::print(data);
}
