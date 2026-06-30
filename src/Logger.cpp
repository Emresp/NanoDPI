#include "..//include/Logger.h"

#include <iostream>
#include <mutex>

namespace
{
    std::mutex logMutex;
}

void logLine(const std::string& message)
{
    std::lock_guard<std::mutex> lock(logMutex);
    std::cout << message << std::endl;
}