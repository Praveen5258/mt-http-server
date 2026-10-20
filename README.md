# Multithreaded HTTP Server in C++ (Windows)

A robust, custom-built Multithreaded HTTP Server using modern C++ and the native Windows Winsock2 API.

## Project Overview

This project demonstrates core Operating Systems and Concurrency concepts by building a web server entirely from scratch without external web or threading libraries. It features a custom thread pool, a thread-safe task queue, backpressure handling, and graceful shutdown.

## Architecture

```text
Main Thread (Winsock accept)
     ↓
Task Queue (std::queue + std::mutex)
     ↓ (std::condition_variable)
Worker Threads (std::thread pool)
     ↓
HTTP Request Parsing
     ↓
HTTP Response (Static file serving)
```

## OS Concepts Demonstrated

- **Threads & Thread Pool**: Pre-allocates worker threads to avoid the overhead of thread creation for each request.
- **Mutex**: `std::mutex` is used to prevent data races when accessing the shared task queue and logger.
- **Condition Variable**: `std::condition_variable` puts worker threads to sleep when idle and wakes them up when new tasks are enqueued, avoiding CPU-wasting busy waits.
- **Producer-Consumer Pattern**: The main thread produces client connection tasks; the worker threads consume them.
- **Backpressure**: The task queue has a configurable maximum size. If the queue is full, the server rejects new tasks and sends a `503 Service Unavailable` response, preventing memory exhaustion.
- **Graceful Shutdown**: Traps `Ctrl+C` to stop accepting new connections, processes pending tasks, and cleanly joins all threads before exiting.

## Networking Concepts

- **TCP/Socket Lifecycle**: Explicitly manages `socket()`, `bind()`, `listen()`, `accept()`, and `closesocket()`.
- **HTTP/1.1**: Implements basic HTTP request parsing, `Content-Length`, `Content-Type`, and connection lifecycle (`keep-alive` vs `close`).

## Build Instructions (Windows)

Prerequisites: CMake and MSVC (Visual Studio).

1. Open a **Developer Command Prompt for VS** (or x64 Native Tools Command Prompt).
2. Navigate to the `mt-http-server` directory.
3. Run CMake:
   ```cmd
   mkdir build
   cd build
   cmake ..
   cmake --build . --config Release
   ```
4. The executable will be generated at `build/Release/server.exe`.

*(Note: Run the server from the root of `mt-http-server` so it can find the `www/` folder)*

## Run

Run the server by passing the Port, Number of Workers, and Max Queue Size:

```cmd
cd ..
build\Release\server.exe 8080 4 100
```

## Testing

Open your browser and navigate to:
[http://localhost:8080](http://localhost:8080)

You can also test with multiple simultaneous clients using tools like `curl`, browser tabs, or benchmarking software.

## Future Improvements (Out of scope)

- Support for POST requests.
- Asynchronous I/O using Windows IOCP (I/O Completion Ports).
- HTTPS via OpenSSL or SChannel.
