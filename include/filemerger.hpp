#pragma once

#include <string>
#include <vector>

#include "chunkcalculator.hpp"

class FileMerger {
public:
    bool mergeFiles(const std::vector<ChunkInfo>& chunks,
                    const std::string& outputName) const;
};
