#pragma once
#include <string>
#include <mutex>
#include <atomic>

struct ServerStats {
    std::atomic<int> totalRequests{0};
    std::atomic<int> successfulRequests{0};
    std::atomic<int> failedRequests{0};
    std::atomic<int> activeConnections{0};
    std::atomic<int> rejectedRequests{0};
};

class Logger {
public:
    static Logger& getInstance();
    
    void log(const std::string& method, const std::string& path, int status, size_t responseSize);
    void logMessage(const std::string& message);
    
    ServerStats stats;

private:
    Logger() = default;
    std::mutex logMutex;
};
