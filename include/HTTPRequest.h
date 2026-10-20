#pragma once
#include <string>
#include <map>
#include <winsock2.h>

struct HTTPRequest {
    std::string method;
    std::string path;
    std::string version;
    std::map<std::string, std::string> headers;
    bool keepAlive = false;
    
    bool parse(SOCKET clientSocket);
};

struct HTTPResponse {
    int statusCode;
    std::string contentType;
    std::string body;
    bool closeConnection;
    
    void sendTo(SOCKET clientSocket) const;
};
