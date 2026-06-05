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
    static bool merge(const std::vector<ChunkInfo>& chunks,
                      const std::string& outputFile);

private:

    // Delete temp files after merge
    static void cleanup(const std::vector<ChunkInfo>& chunks);
};