#pragma once
#include <string>
#include <vector>
#include "chunkcalculator.hpp"

// ─────────────────────────────────────────
// Merges all downloaded chunks into
// a single final output file
// ─────────────────────────────────────────
class FileMerger {
public:

    // Merge chunk files into final output
    // FIX: pre-checks all chunks exist before
    // opening the output file, so a missing chunk
    // never produces a partial output
    static bool merge(const std::vector<ChunkInfo>& chunks,
                      const std::string& outputFile);

private:

    // Verify all temp files exist before merging
    static bool allChunksPresent(
        const std::vector<ChunkInfo>& chunks);

    // Delete temp files after merge
    static void cleanup(const std::vector<ChunkInfo>& chunks);

    // Remove output file on failure
    static void removeOutput(const std::string& outputFile);
};