#include "../include/logger.hpp"
#include <iostream>

Logger::Logger()
{
    logFile.open("logs/download.log",
                 std::ios::app);
}

Logger::~Logger()
{
    if(logFile.is_open())
        logFile.close();
}

Logger& Logger::getInstance()
{
    static Logger instance;
    return instance;
}

void Logger::log(const std::string& msg)
{
    std::lock_guard<std::mutex> lock(logMutex);

    std::cout << msg << std::endl;

    if(logFile.is_open())
    {
        logFile << msg << std::endl;
    }
}
