# Benchmarking the Server

To benchmark this server, you can use tools like `hey`, `ab` (Apache Bench), or `wrk` (via WSL). Since this server is native Windows, `hey` or `ab` are common native choices.

## 1. Setup

Start the server from the command line:
```cmd
server.exe 8080 4 100
```

## 2. Using Apache Bench (`ab`)

If you have Apache Bench installed, run:

```cmd
ab -n 5000 -c 100 http://127.0.0.1:8080/index.html
```
*(This sends 5000 total requests with 100 concurrent clients).*

## 3. Compare Worker Counts

Restart the server with different thread counts and observe the changes in **Requests per second (Req/s)** and **Latency (ms)**. Keep the queue size large enough (e.g., 1000) so you don't reject legitimate benchmarking traffic.

- **1 worker**: `server.exe 8080 1 1000`
- **2 workers**: `server.exe 8080 2 1000`
- **4 workers**: `server.exe 8080 4 1000`
- **8 workers**: `server.exe 8080 8 1000`

### Comparing with Thread-per-request
In a simplistic thread-per-request model, a new thread is spawned for every single connection. Under a heavy load like `ab -c 1000`, the server would attempt to spawn 1000 threads simultaneously. Thread creation overhead and extreme context switching would severely degrade performance, and unbounded thread creation can crash the OS.

Our Thread Pool model pre-allocates threads and uses a bounded queue. This bounds memory and CPU usage, and handles overloads gracefully by rejecting connections (backpressure) instead of crashing.
