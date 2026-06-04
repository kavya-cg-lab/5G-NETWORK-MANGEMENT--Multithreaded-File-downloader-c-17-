#include "ChunkManager.hpp"
#include <vector>
#include <iostream>

std::vector<std::pair<long, long>> divideChunks(long fileSize, int threads) {

    std::vector<std::pair<long, long>> chunks;

    long chunkSize = fileSize / threads;

    for (int i = 0; i < threads; i++) {

        long start = i * chunkSize;

        long end;
        if (i == threads - 1)
            end = fileSize - 1;
        else
            end = start + chunkSize - 1;

        chunks.push_back({start, end});
    }

    return chunks;
}