#include <gtest/gtest.h>
#include <fstream>
#include "filemerger.hpp"

// Helper: create dummy chunk file
void createChunk(const std::string& name,
                 const std::string& content) {

    std::ofstream file(name, std::ios::binary);
    file << content;
    file.close();
}

TEST(FileMergerTest, MergeSuccess) {

    std::vector<ChunkInfo> chunks = {
        {0, 0, 0, 0, "chunk_0.tmp"},
        {1, 0, 0, 0, "chunk_1.tmp"}
    };

    createChunk("chunk_0.tmp", "Hello ");
    createChunk("chunk_1.tmp", "World");

    bool result = FileMerger::merge(chunks, "output.txt");

    EXPECT_TRUE(result);

    std::ifstream file("output.txt");
    std::string content;
    std::getline(file, content);

    EXPECT_EQ(content, "Hello World");

    file.close();

    std::remove("output.txt");
}

TEST(FileMergerTest, MissingChunk) {

    std::vector<ChunkInfo> chunks = {
        {0, 0, 0, 0, "chunk_0.tmp"},
        {1, 0, 0, 0, "chunk_missing.tmp"}
    };

    createChunk("chunk_0.tmp", "Hello");

    bool result = FileMerger::merge(chunks, "output.txt");

    EXPECT_FALSE(result);

    std::remove("chunk_0.tmp");
}