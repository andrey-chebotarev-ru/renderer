#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <queue>
#include <vector>
#include <utility>

class ThreadPool
{
public:
    explicit ThreadPool(size_t count);
    ~ThreadPool();

    void submit(std::function<void()> task);
    void wait();

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;

    std::mutex mutex;
    std::condition_variable taskCV;
    std::condition_variable doneCV;

    size_t pending = 0;
    bool stopping = false;

    void workerLoop();
};