#pragma once

#include <vector>
#include <string>

#include "chunkcalculator.hpp"

// ─────────────────────────────────────────
// Owns the chunk list for a download session
// Adds caching so chunks are only calculated once
// ─────────────────────────────────────────
class ChunkManager {
public:

    // Divide file into chunks and cache the result
    // Throws std::invalid_argument on bad inputs
    const std::vector<ChunkInfo>& divideIntoChunks(long long fileSize,
                                                    int numThreads);

    // Return cached chunks (call divideIntoChunks first)
    const std::vector<ChunkInfo>& getChunks() const;

    // Print current chunk layout to terminal
    void printChunks() const;

    // True if chunks have been calculated
    bool hasChunks() const;

    // Clear cached state (e.g. for a new download)
    void reset();

private:
    std::vector<ChunkInfo> chunks_;
    bool                   ready_ = false;
};