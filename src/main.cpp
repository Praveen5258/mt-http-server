#include "HTTPServer.h"
#include "Logger.h"
#include <iostream>
#include <atomic>
#include <windows.h>
#include <string>

std::atomic<bool> g_stopFlag{false};
HTTPServer* g_server = nullptr;

BOOL WINAPI ConsoleHandler(DWORD signal) {
    if (signal == CTRL_C_EVENT || signal == CTRL_CLOSE_EVENT) {
        std::cout << "\nShutdown signal received. Stopping gracefully...\n";
        g_stopFlag = true;
        if (g_server) {
            g_server->stop(); // Notifies threads to exit cleanly
        }
        return TRUE;
    }
    return FALSE;
}

void printUsage(const char* progName) {
    std::cout << "Usage: " << progName << " <port> <workers> <queue_size>\n"
              << "Example: " << progName << " 8080 4 100\n";
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        printUsage(argv[0]);
        return 1;
    }

    int port = std::stoi(argv[1]);
    size_t workers = std::stoull(argv[2]);
    size_t queueSize = std::stoull(argv[3]);

    if (!SetConsoleCtrlHandler(ConsoleHandler, TRUE)) {
        std::cerr << "Could not set control handler\n";
        return 1;
    }

    try {
        HTTPServer server(port, workers, queueSize);
        g_server = &server;
        
        std::cout << "Starting server on port " << port << " with " << workers 
                  << " workers and queue size " << queueSize << ".\n";
        std::cout << "Press Ctrl+C to stop.\n";
        
        // This blocks until g_server->stop() is called from the console handler
        server.start(); 
        
    } catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << "\n";
    }

    // Print final statistics after threads joined
    ServerStats& stats = Logger::getInstance().stats;
    std::cout << "\n--- Server Statistics ---\n";
    std::cout << "Total Requests:      " << stats.totalRequests << "\n";
    std::cout << "Successful Requests: " << stats.successfulRequests << "\n";
    std::cout << "Failed Requests:     " << stats.failedRequests << "\n";
    std::cout << "Rejected Requests:   " << stats.rejectedRequests << "\n";
    std::cout << "Active Connections:  " << stats.activeConnections << " (should be 0)\n";
    
    return 0;
}
