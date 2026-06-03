#include <gtest/gtest.h>
#include "fileinfo.hpp"

TEST(FileInfoTest, ValidURL) {
    std::string url = "http://speedtest.tele2.net/1MB.zip";

    FileData data = FileInfo::fetch(url);

    EXPECT_TRUE(data.isValid);
    EXPECT_GT(data.fileSize, 0);
    EXPECT_FALSE(data.fileName.empty());
}

TEST(FileInfoTest, RangeSupport) {
    std::string url = "http://speedtest.tele2.net/1MB.zip";

    FileData data = FileInfo::fetch(url);

    EXPECT_TRUE(data.supportsRange);
}

TEST(FileInfoTest, InvalidURL) {
    std::string url = "http://invalid.example.fake/file.zip";

    FileData data = FileInfo::fetch(url);

    EXPECT_FALSE(data.isValid);
}

TEST(FileInfoTest, EmptyURL) {
    std::string url = "";

    FileData data = FileInfo::fetch(url);

    EXPECT_FALSE(data.isValid);
}

TEST(FileInfoTest, FilenameExtraction) {
    std::string url = "http://example.com/files/video.mp4";

    FileData data = FileInfo::fetch(url);

    EXPECT_EQ(data.fileName, "video.mp4");
}

TEST(FileInfoTest, DefaultFilenameFallback) {
    std::string url = "http://example.com/";

    FileData data = FileInfo::fetch(url);

    EXPECT_EQ(data.fileName, "downloaded_file");
}

TEST(FileInfoTest, FileSizePositive) {
    std::string url = "http://speedtest.tele2.net/1MB.zip";
 
    FileData data = FileInfo::fetch(url);

    if (!data.isValid) {
        GTEST_SKIP() << "Skipping due to network timeout";
    }

    EXPECT_GT(data.fileSize, 0);

}