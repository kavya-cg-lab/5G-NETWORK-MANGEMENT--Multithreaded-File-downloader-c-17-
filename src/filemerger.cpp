#include "../include/filemerger.hpp"
#include <iostream>
#include <fstream>
#include <cstdio>   // std::remove

// ─────────────────────────────────────────
// Verify every chunk file exists on disk
// FIX: run this BEFORE opening the output
// file so we never produce a partial result
// ─────────────────────────────────────────
bool FileMerger::allChunksPresent(
        const std::vector<ChunkInfo>& chunks) {

    for (const auto& chunk : chunks) {
        std::ifstream f(chunk.tempFile,
                        std::ios::binary);
        if (!f) {
            std::cerr << "Missing chunk: "
                      << chunk.tempFile << "\n";
            return false;
        }
    }
    return true;
}

// ─────────────────────────────────────────
// Remove the output file if merge failed
// FIX: prevents leaving a partial file
// ─────────────────────────────────────────
void FileMerger::removeOutput(
        const std::string& outputFile) {

    if (std::remove(outputFile.c_str()) == 0) {
        std::cout << "Partial output removed: "
                  << outputFile << "\n";
    }
}

// ─────────────────────────────────────────
// Merge all chunk files in order
// ─────────────────────────────────────────
bool FileMerger::merge(const std::vector<ChunkInfo>& chunks,
                       const std::string& outputFile) {

    // FIX: pre-check before touching the output file
    if (!allChunksPresent(chunks)) {
        std::cerr << "Merge aborted: one or more chunks missing\n";
        return false;
    }

    std::ofstream output(outputFile, std::ios::binary);
    if (!output) {
        std::cerr << "Failed to create output file: "
                  << outputFile << "\n";
        return false;
    }

    for (const auto& chunk : chunks) {

        std::ifstream input(chunk.tempFile,
                            std::ios::binary);
        if (!input) {
            // Should not happen after pre-check,
            // but handle race conditions defensively
            std::cerr << "Cannot read chunk: "
                      << chunk.tempFile << "\n";
            output.close();
            removeOutput(outputFile);
            return false;
        }

        output << input.rdbuf();

        if (!output) {
            std::cerr << "Write error while merging: "
                      << chunk.tempFile << "\n";
            output.close();
            removeOutput(outputFile);
            return false;
        }

        input.close();
    }

    output.close();

    std::cout << "File merged successfully: "
              << outputFile << "\n";

    cleanup(chunks);
    return true;
}

// ─────────────────────────────────────────
// Delete temporary chunk files
// ─────────────────────────────────────────
void FileMerger::cleanup(
        const std::vector<ChunkInfo>& chunks) {

    for (const auto& chunk : chunks) {
        std::remove(chunk.tempFile.c_str());
    }
    std::cout << "Temporary files cleaned\n";
}