#include "../include/chunkcalculator.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

std::vector<ChunkInfo> ChunkCalculator::calculate(long long fileSize,
                                                   int numThreads) {
    if (numThreads <= 0)
        throw std::invalid_argument("numThreads must be > 0");
    if (fileSize <= 0)
        throw std::invalid_argument("fileSize must be > 0");

    std::vector<ChunkInfo> chunks;
    chunks.reserve(numThreads);
    long long base = fileSize / numThreads;

    for (int i = 0; i < numThreads; ++i) {
        ChunkInfo c;
        c.threadId  = i;
        c.startByte = static_cast<long long>(i) * base;
        c.endByte   = (i == numThreads - 1) ? fileSize - 1
                                             : c.startByte + base - 1;
        c.chunkSize = c.endByte - c.startByte + 1;
        c.tempFile  = "chunk_" + std::to_string(i) + ".tmp";
        chunks.push_back(c);
    }
    return chunks;
}

// ─────────────────────────────────────────
// CHUNK BREAKDOWN — spec format:
// Thread 0 | 0 → 26214399 | 25 MB
// ─────────────────────────────────────────
void ChunkCalculator::print(const std::vector<ChunkInfo>& chunks) {

    std::cout
        << "\n┌─────────────────────────────────────────────────────┐\n"
        << "│               CHUNK BREAKDOWN                       │\n"
        << "├──────────┬───────────────────────────┬──────────────┤\n"
        << "│  Thread  │        Byte Range         │     Size     │\n"
        << "├──────────┼───────────────────────────┼──────────────┤\n";

    for (const auto& c : chunks) {
        // Range string: "0 → 26214399"
        std::ostringstream range;
        range << c.startByte << " \u2192 " << c.endByte;

        // Size string: "25.00 MB"
        std::ostringstream sz;
        sz << std::fixed << std::setprecision(2)
           << (c.chunkSize / (1024.0 * 1024.0)) << " MB";

        std::cout
            << "│  Thread " << c.threadId << " "
            << "│ " << std::left << std::setw(25) << range.str()
            << " │ " << std::setw(12) << sz.str() << " │\n";
    }

    std::cout
        << "└──────────┴───────────────────────────┴──────────────┘\n";
}