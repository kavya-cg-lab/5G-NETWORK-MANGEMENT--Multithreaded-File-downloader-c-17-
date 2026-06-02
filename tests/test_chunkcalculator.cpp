#include <gtest/gtest.h>
#include "../include/chunkcalculator.hpp"

// ✅ Test 1
TEST(ChunkCalculatorTest, EqualDivision) {
    long long fileSize = 1000;
    int threads = 4;

    auto chunks = ChunkCalculator::calculate(fileSize, threads);

    ASSERT_EQ(chunks.size(), 4);
    EXPECT_EQ(chunks[0].startByte, 0);
    EXPECT_EQ(chunks[3].endByte, 999);
}

// ✅ Test 2
TEST(ChunkCalculatorTest, UnevenDivision) {
    long long fileSize = 1000;
    int threads = 3;

    auto chunks = ChunkCalculator::calculate(fileSize, threads);

    ASSERT_EQ(chunks.size(), 3);
    EXPECT_EQ(chunks[2].chunkSize, 334);
}

// ✅ Test 3
TEST(ChunkCalculatorTest, SingleThread) {
    auto chunks = ChunkCalculator::calculate(500, 1);

    ASSERT_EQ(chunks.size(), 1);
    EXPECT_EQ(chunks[0].startByte, 0);
    EXPECT_EQ(chunks[0].endByte, 499);
}

// ✅ Test 4
TEST(ChunkCalculatorTest, ContinuousCoverage) {
    auto chunks = ChunkCalculator::calculate(1000, 4);

    for (size_t i = 0; i < chunks.size() - 1; i++) {
        EXPECT_EQ(chunks[i].endByte + 1,
                  chunks[i + 1].startByte);
    }
}
