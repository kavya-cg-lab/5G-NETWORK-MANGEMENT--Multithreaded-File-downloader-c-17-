#include "../include/filemerger.hpp"
#include <iostream>
#include <fstream>
#include <cstdio> // for std::remove

// ─────────────────────────────────────────
// Merge all chunk files in order
// ─────────────────────────────────────────
bool FileMerger::merge(const std::vector<ChunkInfo>& chunks,
                       const std::string& outputFile) {

    std::ofstream output(outputFile, std::ios::binary);

    if (!output) {
        std::cerr << "❌ Failed to create output file\n";
        return false;
    }

    for (const auto& chunk : chunks) {

        std::ifstream input(chunk.tempFile, std::ios::binary);

        if (!input) {
            std::cerr << "❌ Missing chunk: " << chunk.tempFile << "\n";
            return false;
        }

        output << input.rdbuf();

        input.close();
    }

    output.close();

    std::cout << "✅ File merged successfully into: "
              << outputFile << "\n";

    cleanup(chunks);

    return true;
}

// ─────────────────────────────────────────
// Delete temporary chunk files
// ─────────────────────────────────────────
void FileMerger::cleanup(const std::vector<ChunkInfo>& chunks) {

    for (const auto& chunk : chunks) {
        std::remove(chunk.tempFile.c_str());
    }

    std::cout << "🧹 Temporary files cleaned\n";
}
