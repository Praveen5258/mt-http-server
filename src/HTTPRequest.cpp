#include "HTTPRequest.h"
#include <sstream>
#include <iostream>

bool HTTPRequest::parse(SOCKET clientSocket) {
    char buffer[4096];
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    
    if (bytesReceived <= 0) {
        return false; // Connection closed or error
    }
    buffer[bytesReceived] = '\0';
    
    std::string requestData(buffer);
    std::istringstream stream(requestData);
    std::string line;
    
    // Parse Request Line
    if (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        std::istringstream lineStream(line);
        lineStream >> method >> path >> version;
    }
    
    // Parse Headers
    while (std::getline(stream, line) && line != "\r") {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string key = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 1);
            
            // Trim leading space
            size_t start = value.find_first_not_of(" ");
            if (start != std::string::npos) {
                value = value.substr(start);
            }
            headers[key] = value;
            
            // Convert key to lowercase for case-insensitive check
            std::string lowerKey = key;
            for(auto& c : lowerKey) c = tolower(c);
            
            if (lowerKey == "connection" && value == "keep-alive") {
                keepAlive = true;
            }
        }
    }
    
    // Default Keep-Alive behavior
    if (version == "HTTP/1.1" && headers["Connection"] != "close") {
        keepAlive = true;
    } else if (version == "HTTP/1.0" && headers["Connection"] == "keep-alive") {
        keepAlive = true;
    } else {
        keepAlive = false;
    }
    
    return !method.empty() && !path.empty();
}

std::string getStatusMessage(int code) {
    switch (code) {
        case 200: return "OK";
        case 400: return "Bad Request";
        case 404: return "Not Found";
        case 500: return "Internal Server Error";
        case 503: return "Service Unavailable";
        default: return "Unknown";
    }
}

void HTTPResponse::sendTo(SOCKET clientSocket) const {
    std::ostringstream responseStream;
    responseStream << "HTTP/1.1 " << statusCode << " " << getStatusMessage(statusCode) << "\r\n";
    responseStream << "Content-Type: " << contentType << "\r\n";
    responseStream << "Content-Length: " << body.size() << "\r\n";
    
    if (closeConnection) {
        responseStream << "Connection: close\r\n";
    } else {
        responseStream << "Connection: keep-alive\r\n";
    }
    
    responseStream << "\r\n";
    responseStream << body;
    
    std::string responseString = responseStream.str();
    send(clientSocket, responseString.c_str(), responseString.size(), 0);
}
