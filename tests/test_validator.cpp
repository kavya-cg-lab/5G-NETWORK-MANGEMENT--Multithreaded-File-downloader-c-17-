#include <gtest/gtest.h>
#include "validator.hpp"

TEST(ValidatorTest, ValidURL) {
    EXPECT_TRUE(Validator::isValidUrl("http://example.com/file.zip"));
    EXPECT_TRUE(Validator::isValidUrl("https://example.com"));
}

TEST(ValidatorTest, InvalidURL) {
    EXPECT_FALSE(Validator::isValidUrl(""));
    EXPECT_FALSE(Validator::isValidUrl("ftp://example.com"));
    EXPECT_FALSE(Validator::isValidUrl("invalid_url"));
}

TEST(ValidatorTest, ValidThreadCount) {
    EXPECT_TRUE(Validator::isValidThreadCount(1));
    EXPECT_TRUE(Validator::isValidThreadCount(8));
    EXPECT_TRUE(Validator::isValidThreadCount(16));
}

TEST(ValidatorTest, InvalidThreadCount) {
    EXPECT_FALSE(Validator::isValidThreadCount(0));
    EXPECT_FALSE(Validator::isValidThreadCount(17));
    EXPECT_FALSE(Validator::isValidThreadCount(-5));
}

TEST(ValidatorTest, ValidOutputName) {
    EXPECT_TRUE(Validator::isValidOutputName("file.zip"));
    EXPECT_TRUE(Validator::isValidOutputName("video.mp4"));
    EXPECT_TRUE(Validator::isValidOutputName("data_123.txt"));
}

TEST(ValidatorTest, InvalidOutputName) {
    EXPECT_FALSE(Validator::isValidOutputName(""));
    EXPECT_FALSE(Validator::isValidOutputName("file/name.txt"));
    EXPECT_FALSE(Validator::isValidOutputName("file*name.txt"));
    EXPECT_FALSE(Validator::isValidOutputName("file?name.txt"));
}

TEST(ValidatorTest, ValidateAllSuccess) {
    EXPECT_TRUE(
        Validator::validateAll(
            "http://example.com/file.zip",
            4,
            "output.zip"
        )
    );
}

TEST(ValidatorTest, ValidateAllFail) {
    EXPECT_FALSE(
        Validator::validateAll(
            "invalid_url",
            4,
            "output.zip"
        )
    );

    EXPECT_FALSE(
        Validator::validateAll(
            "http://example.com/file.zip",
            20,
            "output.zip"
        )
    );

    EXPECT_FALSE(
        Validator::validateAll(
            "http://example.com/file.zip",
            4,
            "file/name.txt"
        )
    );
}

