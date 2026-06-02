#pragma once

#include <string>
#include <vector>
#include <memory>

#include "fileinfomanager.hpp"
#include "chunkmanager.hpp"
#include "downloadsystem.hpp"
#include "validator.hpp"

class MainController {
public:
    MainController(std::string url,
                   int threadCount,
                   std::string outputName);
    ~MainController();

    bool initializeSystem();
    bool startDownload();

private:
    bool validateInputs();
    bool fetchMetadata(FileData& data);
    bool prepareChunks(const FileData& data,
                       std::vector<ChunkInfo>& chunks);
    bool performDownload(const FileData& data,
                         std::vector<ChunkInfo>& chunks);

    template <typename Fn>
    bool retryStep(const std::string& stepName, Fn&& step);

private:
    std::string url;
    int threadCount;
    std::string outputName;

    FileInfoManager fileInfoManager;
    ChunkManager chunkManager;
    static constexpr int maxRetryCount = 3;
};
