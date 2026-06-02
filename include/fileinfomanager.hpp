#pragma once

#include <string>

#include "fileinfo.hpp"

class FileInfoManager {
public:
    FileData fetchFileInfo(const std::string& url) const;
    void printFileInfo(const FileData& data) const;
};
