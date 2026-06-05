#include "../include/chunkmanager.hpp"

std::vector<ChunkInfo> ChunkManager::divideIntoChunks(long long fileSize,
                                                     int numThreads) const {
    return ChunkCalculator::calculate(fileSize, numThreads);
}

void ChunkManager::printChunks(const std::vector<ChunkInfo>& chunks) const {
    ChunkCalculator::print(chunks);
}