#pragma once
#include <string>
#include <winsock2.h>
#include "ThreadPool.h"

class HTTPServer {
public:
    HTTPServer(int port, size_t numThreads, size_t maxQueueSize);
    ~HTTPServer();
    
    void start();
    void stop();

private:
    void handleConnection(SOCKET clientSocket);
    std::string getMimeType(const std::string& path);
    bool serveStaticFile(const std::string& path, std::string& outBody, std::string& outMimeType);

    int port;
    SOCKET listenSocket;
    ThreadPool threadPool;
    std::atomic<bool> isRunning;
    
    const std::string wwwDir = "www";
};
