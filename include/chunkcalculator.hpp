#pragma once
#include <string>
#include <vector>
#include <stdexcept>

// ─────────────────────────────────────────
// Holds byte range info for one thread
// ─────────────────────────────────────────
struct ChunkInfo {
    int       threadId;
    long long startByte;
    long long endByte;
    long long chunkSize;
    std::string tempFile;
};

// ─────────────────────────────────────────
// Splits file size into equal byte ranges
// one range assigned per thread
// ─────────────────────────────────────────
class ChunkCalculator {
public:

    // Calculate all chunk ranges
    // Throws std::invalid_argument if numThreads <= 0 or fileSize <= 0
    static std::vector<ChunkInfo> calculate(long long fileSize,
                                             int numThreads);

    // Print chunk breakdown to terminal
    static void print(const std::vector<ChunkInfo>& chunks);
};