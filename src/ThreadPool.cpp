#include "ThreadPool.h"

ThreadPool::ThreadPool(size_t numThreads, size_t maxQueueSize) 
    : maxQueueSize(maxQueueSize), stop(false) {
    
    for (size_t i = 0; i < numThreads; ++i) {
        workers.emplace_back([this] {
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(this->queueMutex);
                    
                    // Sleep until there is a task or shutdown is requested
                    this->condition.wait(lock, [this] {
                        return this->stop || !this->tasks.empty();
                    });
                    
                    if (this->stop && this->tasks.empty()) {
                        return; // Exit thread
                    }
                    
                    task = std::move(this->tasks.front());
                    this->tasks.pop();
                }
                
                // Execute the task outside the lock to avoid blocking the queue
                task();
            }
        });
    }
}

bool ThreadPool::enqueue(std::function<void()> task) {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        if (stop) {
            return false;
        }
        
        // Backpressure: reject if the queue is full
        if (tasks.size() >= maxQueueSize) {
            return false;
        }
        
        tasks.push(std::move(task));
    }
    
    // Wake up one worker thread
    condition.notify_one();
    return true;
}

void ThreadPool::shutdown() {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        stop = true;
    }
    
    // Wake up all workers so they can exit
    condition.notify_all();
    
    for (std::thread& worker : workers) {
        if (worker.joinable()) {
            worker.join(); // Wait for thread to finish
        }
    }
}

ThreadPool::~ThreadPool() {
    if (!stop) {
        shutdown();
    }
}
