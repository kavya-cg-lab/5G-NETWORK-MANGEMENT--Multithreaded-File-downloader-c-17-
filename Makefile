CXX     = g++
CXXFLAGS = -std=c++17 -Wall -Wextra
LIBS    = -lcurl

SRC     = src/main.cpp \
          src/maincontroller.cpp \
          src/fileinfomanager.cpp \
          src/chunkmanager.cpp \
          src/downloadsystem.cpp \
          src/progresstracker.cpp \
          src/filemerger.cpp \
          src/validator.cpp \
          src/fileinfo.cpp \
          src/chunkcalculator.cpp \
          src/ui.cpp

TARGET  = downloader

all:
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC) $(LIBS)

clean:
	rm -f $(TARGET) *.tmp
