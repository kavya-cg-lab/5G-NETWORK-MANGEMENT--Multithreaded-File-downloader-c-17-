CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude
LIBS = -lcurl

SRC = src/main.cpp \
      src/validator.cpp \
      src/fileinfo.cpp \
      src/chunkcalculator.cpp \
      src/downloader.cpp

TARGET = downloader

# ✅ ChunkCalculator test
TEST_SRC = tests/test_chunkcalculator.cpp src/chunkcalculator.cpp
TEST_TARGET = test_chunks
TEST_LIBS = -lgtest -lgtest_main -pthread

# ✅ FileInfo test
TEST_FILEINFO_SRC = tests/test_fileinfo.cpp src/fileinfo.cpp
TEST_FILEINFO_TARGET = test_fileinfo

TEST_VALIDATOR_SRC = tests/test_validator.cpp src/validator.cpp
TEST_VALIDATOR_TARGET = test_validator

# ─────────────────────────────
# Main build
# ─────────────────────────────
all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC) $(LIBS)

# ─────────────────────────────
# Chunk tests
# ─────────────────────────────
test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): $(TEST_SRC)
	$(CXX) $(CXXFLAGS) -o $(TEST_TARGET) $(TEST_SRC) $(TEST_LIBS)

# ─────────────────────────────
# FileInfo tests
# ─────────────────────────────
test_fileinfo: $(TEST_FILEINFO_TARGET)
	./$(TEST_FILEINFO_TARGET)

$(TEST_FILEINFO_TARGET): $(TEST_FILEINFO_SRC)
	$(CXX) $(CXXFLAGS) -o $(TEST_FILEINFO_TARGET) \
	$(TEST_FILEINFO_SRC) $(TEST_LIBS) $(LIBS)

test_validator: $(TEST_VALIDATOR_TARGET)
	./$(TEST_VALIDATOR_TARGET)

$(TEST_VALIDATOR_TARGET): $(TEST_VALIDATOR_SRC)
	$(CXX) $(CXXFLAGS) -o $(TEST_VALIDATOR_TARGET) $(TEST_VALIDATOR_SRC) $(TEST_LIBS)	

# Run all tests
test_all: $(TEST_TARGET) $(TEST_FILEINFO_TARGET)
	./$(TEST_TARGET)
	./$(TEST_FILEINFO_TARGET)


# ─────────────────────────────
# Clean
# ─────────────────────────────
clean:
	rm -f $(TARGET) $(TEST_TARGET) $(TEST_FILEINFO_TARGET) *.tmp
