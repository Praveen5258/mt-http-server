#include "Logger.h"
#include <iostream>
#include <chrono>

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

std::string getCurrentTime() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    struct tm timeinfo;
    localtime_s(&timeinfo, &now_time);
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%H:%M:%S", &timeinfo);
    return std::string(buffer);
}

void Logger::log(const std::string& method, const std::string& path, int status, size_t responseSize) {
    std::lock_guard<std::mutex> lock(logMutex);
    std::cout << "[" << getCurrentTime() << "] " 
              << method << " " << path << " " 
              << status << " " << responseSize << " bytes\n";
}

void Logger::logMessage(const std::string& message) {
    std::lock_guard<std::mutex> lock(logMutex);
    std::cout << "[" << getCurrentTime() << "] " << message << "\n";
}
