#pragma once

#include <vector>
#include <string>

#include "chunkcalculator.hpp"

class ChunkManager {
public:
    std::vector<ChunkInfo> divideIntoChunks(long long fileSize,
                                            int numThreads) const;

    void printChunks(const std::vector<ChunkInfo>& chunks) const;
};
