#include <gtest/gtest.h>
#include <fstream>
#include <cstdio>   // for std::remove
#include "downloader.hpp"
#include "chunkcalculator.hpp"

TEST(DownloaderTest, SingleThreadDownload) {
    std::string url ="http://speedtest.tele2.net/10GB.zip";

    auto chunks = ChunkCalculator::calculate(100, 1);

    Downloader::downloadAll(url, chunks);

    std::ifstream file("chunk_0.tmp", std::ios::binary);

    ASSERT_TRUE(file.good());

    file.seekg(0, std::ios::end);
    EXPECT_GT(file.tellg(), 0);

    file.close();

    std::remove("chunk_0.tmp");
}

TEST(DownloaderTest, MultiThreadDownload) {
    std::string url = "http://speedtest.tele2.net/10GB.zip";

    auto chunks = ChunkCalculator::calculate(200, 3);

    Downloader::downloadAll(url, chunks);

    for (int i = 0; i < 3; i++) {
        std::string filename = "chunk_" + std::to_string(i) + ".tmp";

        std::ifstream file(filename, std::ios::binary);
        EXPECT_TRUE(file.good());

        file.seekg(0, std::ios::end);
        EXPECT_GT(file.tellg(), 0);

        file.close();
        std::remove(filename.c_str());
    }
}

TEST(DownloaderTest, InvalidURL) {
    std::string url = "http://invalid.url.test/file.txt";

    auto chunks = ChunkCalculator::calculate(100, 2);

    Downloader::downloadAll(url, chunks);

    for (int i = 0; i < 2; i++) {
        std::string filename = "chunk_" + std::to_string(i) + ".tmp";

        std::ifstream file(filename);
        EXPECT_TRUE(file.good()); // file may exist but be empty

        file.close();
        std::remove(filename.c_str());
    }
}

TEST(DownloaderTest, FileSizeNonZero) {
    std::string url = "http://speedtest.tele2.net/10GB.zip";

    auto chunks = ChunkCalculator::calculate(300, 2);

    Downloader::downloadAll(url, chunks);

    long totalSize = 0;

    for (int i = 0; i < 2; i++) {
        std::string filename = "chunk_" + std::to_string(i) + ".tmp";

        std::ifstream file(filename, std::ios::binary);
        file.seekg(0, std::ios::end);
        totalSize += file.tellg();

        file.close();
        std::remove(filename.c_str());
    }

    EXPECT_GT(totalSize, 0);
}

