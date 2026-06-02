#include "../include/filemerger.hpp"
#include <fstream>
#include <iostream>

bool FileMerger::mergeFiles(const std::vector<ChunkInfo>& chunks,
                            const std::string& outputName) const {
    std::ofstream output(outputName, std::ios::binary);
    if (!output) {
        std::cerr << "❌ Could not open output file: " << outputName << "\n";
        return false;
    }

    constexpr size_t bufferSize = 8 * 1024;
    std::vector<char> buffer(bufferSize);

    for (const auto& chunk : chunks) {
        std::ifstream input(chunk.tempFile, std::ios::binary);
        if (!input) {
            std::cerr << "❌ Could not open temp chunk: "
                      << chunk.tempFile << "\n";
            return false;
        }

        while (input) {
            input.read(buffer.data(), buffer.size());
            std::streamsize bytesRead = input.gcount();
            if (bytesRead > 0) {
                output.write(buffer.data(), bytesRead);
                if (!output) {
                    std::cerr << "❌ Failed while writing to output file\n";
                    return false;
                }
            }
        }
    }

    return true;
}
