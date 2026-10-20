#pragma once
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>

class ThreadPool {
public:
    ThreadPool(size_t numThreads, size_t maxQueueSize);
    ~ThreadPool();
    
    // Returns false if queue is full (Backpressure handling)
    bool enqueue(std::function<void()> task);
    
    // Initiate graceful shutdown
    void shutdown();

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    
    std::mutex queueMutex;
    std::condition_variable condition;
    
    size_t maxQueueSize;
    std::atomic<bool> stop;
};
