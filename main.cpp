#include <iostream>
#include "core/FileInfoManager.hpp"
#include "core/ChunkManager.hpp"

int main() {

    std::string url = "https://raw.githubusercontent.com/github/gitignore/main/C++.gitignore";

    long fileSize = getFileSize(url);

    int threads = 4;

    auto chunks = divideChunks(fileSize, threads);

    std::cout << "File size: " << fileSize << "\n";

    for (int i = 0; i < chunks.size(); i++) {
        std::cout << "Chunk " << i << ": "
                  << chunks[i].first
                  << " to "
                  << chunks[i].second << "\n";
    }

    return 0;
}
