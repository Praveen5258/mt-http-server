#include "HTTPServer.h"
#include "HTTPRequest.h"
#include "Logger.h"
#include <iostream>
#include <fstream>
#include <sstream>

HTTPServer::HTTPServer(int port, size_t numThreads, size_t maxQueueSize)
    : port(port), threadPool(numThreads, maxQueueSize), isRunning(false), listenSocket(INVALID_SOCKET) {
    
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        throw std::runtime_error("WSAStartup failed");
    }
}

HTTPServer::~HTTPServer() {
    stop();
    WSACleanup();
}

void HTTPServer::start() {
    listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {
        throw std::runtime_error("Socket creation failed");
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        closesocket(listenSocket);
        throw std::runtime_error("Bind failed");
    }

    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        closesocket(listenSocket);
        throw std::runtime_error("Listen failed");
    }

    isRunning = true;
    Logger::getInstance().logMessage("Server started on port " + std::to_string(port));

    while (isRunning) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(listenSocket, &readfds);

        // 1-second timeout to allow checking isRunning flag
        timeval timeout = {1, 0};
        
        int activity = select(0, &readfds, NULL, NULL, &timeout);
        
        if (activity < 0) {
            if (isRunning) {
                Logger::getInstance().logMessage("Select error");
            }
            break;
        }

        if (activity > 0 && FD_ISSET(listenSocket, &readfds)) {
            SOCKET clientSocket = accept(listenSocket, NULL, NULL);
            if (clientSocket != INVALID_SOCKET) {
                
                // Add timeouts so worker threads don't block forever on read
                DWORD socketTimeout = 5000; // 5 seconds
                setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&socketTimeout, sizeof(socketTimeout));
                setsockopt(clientSocket, SOL_SOCKET, SO_SNDTIMEO, (const char*)&socketTimeout, sizeof(socketTimeout));

                // Enqueue task to thread pool
                bool enqueued = threadPool.enqueue([this, clientSocket]() {
                    this->handleConnection(clientSocket);
                });

                if (!enqueued) {
                    // Backpressure in action
                    Logger::getInstance().stats.rejectedRequests++;
                    
                    // Respond with 503 Service Unavailable directly on main thread
                    HTTPResponse response{503, "text/plain", "Server is too busy", true};
                    response.sendTo(clientSocket);
                    closesocket(clientSocket);
                }
            }
        }
    }
    
    if (listenSocket != INVALID_SOCKET) {
        closesocket(listenSocket);
        listenSocket = INVALID_SOCKET;
    }
}

void HTTPServer::stop() {
    if (isRunning) {
        isRunning = false; // This breaks the select() loop
        Logger::getInstance().logMessage("Initiating graceful shutdown...");
        threadPool.shutdown(); // Wait for existing connections to finish
    }
}

std::string HTTPServer::getMimeType(const std::string& path) {
    size_t dotPos = path.find_last_of('.');
    if (dotPos != std::string::npos) {
        std::string ext = path.substr(dotPos);
        if (ext == ".html") return "text/html";
        if (ext == ".css") return "text/css";
        if (ext == ".js") return "application/javascript";
        if (ext == ".txt") return "text/plain";
    }
    return "application/octet-stream";
}

bool HTTPServer::serveStaticFile(const std::string& path, std::string& outBody, std::string& outMimeType) {
    std::string safePath = path == "/" ? "/index.html" : path;
    
    // Basic protection against directory traversal
    if (safePath.find("..") != std::string::npos) {
        return false;
    }

    std::string fullPath = wwwDir + safePath;
    std::ifstream file(fullPath, std::ios::binary);
    
    if (!file.is_open()) {
        return false;
    }

    std::ostringstream ss;
    ss << file.rdbuf();
    outBody = ss.str();
    outMimeType = getMimeType(safePath);
    return true;
}

void HTTPServer::handleConnection(SOCKET clientSocket) {
    Logger& logger = Logger::getInstance();
    logger.stats.activeConnections++;
    
    bool keepAlive = true;
    
    while (keepAlive && isRunning) {
        HTTPRequest req;
        if (!req.parse(clientSocket)) {
            break; // Connection closed or error
        }
        
        logger.stats.totalRequests++;
        
        HTTPResponse res;
        res.closeConnection = !req.keepAlive;
        keepAlive = req.keepAlive;
        
        if (req.method != "GET") {
            res.statusCode = 400;
            res.contentType = "text/plain";
            res.body = "Bad Request: Only GET is supported.";
            logger.stats.failedRequests++;
        } else {
            std::string body;
            std::string mimeType;
            if (serveStaticFile(req.path, body, mimeType)) {
                res.statusCode = 200;
                res.contentType = mimeType;
                res.body = body;
                logger.stats.successfulRequests++;
            } else {
                res.statusCode = 404;
                res.contentType = "text/plain";
                res.body = "404 Not Found";
                logger.stats.failedRequests++;
            }
        }
        
        res.sendTo(clientSocket);
        logger.log(req.method, req.path, res.statusCode, res.body.size());
    }
    
    closesocket(clientSocket);
    logger.stats.activeConnections--;
}
