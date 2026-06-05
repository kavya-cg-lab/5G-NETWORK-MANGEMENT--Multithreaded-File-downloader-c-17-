CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude
LIBS = -lcurl

SRC = src/main.cpp \
      src/validator.cpp \
      src/fileinfo.cpp \
	  src/fileinfomanager.cpp \
      src/chunkcalculator.cpp \
	  src/chunkmanager.cpp \
      src/downloader.cpp \
      src/filemerger.cpp 
	  

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

TEST_DOWNLOADER_SRC = tests/test_downloader.cpp \
		      src/downloader.cpp \
                      src/chunkcalculator.cpp
TEST_DOWNLOADER_TARGET = test_downloader

TEST_FILEMERGER_SRC = tests/test_filemerger.cpp \
		      src/filemerger.cpp
TEST_FILEMERGER_TARGET = test_filemerger

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

test_downloader: $(TEST_DOWNLOADER_TARGET)
	./$(TEST_DOWNLOADER_TARGET)

$(TEST_DOWNLOADER_TARGET): $(TEST_DOWNLOADER_SRC)
	$(CXX) $(CXXFLAGS) -o $(TEST_DOWNLOADER_TARGET) \
	$(TEST_DOWNLOADER_SRC) $(TEST_LIBS) $(LIBS)

test_filemerger: $(TEST_FILEMERGER_TARGET)
	./$(TEST_FILEMERGER_TARGET)

$(TEST_FILEMERGER_TARGET): $(TEST_FILEMERGER_SRC)
	$(CXX) $(CXXFLAGS) -o $(TEST_FILEMERGER_TARGET) \
	$(TEST_FILEMERGER_SRC) $(TEST_LIBS)	
		

# Run all tests
test_all: $(TEST_TARGET) $(TEST_FILEINFO_TARGET) $(TEST_VALIDATOR_TARGET) $(TEST_DOWNLOADER_TARGET) $(TEST_FILEMERGER_TARGET)
	./$(TEST_TARGET)
	./$(TEST_FILEINFO_TARGET)
	./$(TEST_VALIDATOR_TARGET)
	./$(TEST_DOWNLOADER_TARGET)
	./$(TEST_FILEMERGER_TARGET)


# ─────────────────────────────
# Clean
# ─────────────────────────────
clean:
	rm -f $(TARGET) $(TEST_TARGET) $(TEST_FILEINFO_TARGET) $(TEST_VALIDATOR_TARGET) $(TEST_DOWNLOADER_TARGET) $(TEST_FILEMERGER_TARGET) *.tmp
