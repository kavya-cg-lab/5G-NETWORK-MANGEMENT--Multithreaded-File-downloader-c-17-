#pragma once

#include <fstream>
#include <mutex>
#include <string>

class Logger
{
private:

    std::ofstream logFile;
    std::mutex logMutex;

    Logger();

public:

    static Logger& getInstance();

    void log(const std::string& msg);

    ~Logger();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
};
