#include "../include/chunkmanager.hpp"
#include <stdexcept>

// ─────────────────────────────────────────
// Calculate and cache chunks
// ─────────────────────────────────────────
const std::vector<ChunkInfo>& ChunkManager::divideIntoChunks(
        long long fileSize, int numThreads) {

    chunks_ = ChunkCalculator::calculate(fileSize, numThreads);
    ready_  = true;
    return chunks_;
}

// ─────────────────────────────────────────
// Return cached chunks
// ─────────────────────────────────────────
const std::vector<ChunkInfo>& ChunkManager::getChunks() const {

    if (!ready_) {
        throw std::logic_error(
            "No chunks available — call divideIntoChunks first");
    }
    return chunks_;
}

// ─────────────────────────────────────────
// Print using ChunkCalculator formatter
// ─────────────────────────────────────────
void ChunkManager::printChunks() const {

    if (!ready_) {
        throw std::logic_error(
            "No chunks to print — call divideIntoChunks first");
    }
    ChunkCalculator::print(chunks_);
}

bool ChunkManager::hasChunks() const {
    return ready_;
}

void ChunkManager::reset() {
    chunks_.clear();
    ready_ = false;
}