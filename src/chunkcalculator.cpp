#include "../include/chunkcalculator.hpp"
#include <iostream>
#include <iomanip>

// ─────────────────────────────────────────
// Divide file into equal byte ranges
// Last chunk absorbs any remainder bytes
// ─────────────────────────────────────────
std::vector<ChunkInfo> ChunkCalculator::calculate(
                            long long fileSize,
                            int numThreads) {

    std::vector<ChunkInfo> chunks;

    // Base size for each chunk
    long long chunkSize = fileSize / numThreads;

    for (int i = 0; i < numThreads; i++) {

        ChunkInfo chunk;

        chunk.threadId  = i;
        chunk.startByte = i * chunkSize;

        // Last thread gets all remaining bytes
        if (i == numThreads - 1) {
            chunk.endByte = fileSize - 1;
        } else {
            chunk.endByte = chunk.startByte + chunkSize - 1;
        }

        chunk.chunkSize = chunk.endByte
                        - chunk.startByte + 1;

        // Temp file name for this thread
        chunk.tempFile  = "chunk_"
                        + std::to_string(i)
                        + ".tmp";

        chunks.push_back(chunk);
    }

    return chunks;
}

// ─────────────────────────────────────────
// Print each thread's byte range clearly
// ─────────────────────────────────────────
void ChunkCalculator::print(
         const std::vector<ChunkInfo>& chunks) {

    std::cout << "\n================================\n";
    std::cout << "       CHUNK BREAKDOWN          \n";
    std::cout << "================================\n";

    for (const auto& chunk : chunks) {

        std::cout << "Thread " << chunk.threadId
                  << " | Bytes: "
                  << std::setw(10) << chunk.startByte
                  << " → "
                  << std::setw(10) << chunk.endByte
                  << " | Size: "
                  << std::setw(6)
                  << chunk.chunkSize / 1024
                  << " KB"
                  << " | Temp: "
                  << chunk.tempFile
                  << "\n";
    }

    std::cout << "================================\n";
}