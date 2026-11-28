#include "ThreadPool.h"

ThreadPool::ThreadPool(size_t count)
{
    if (count == 0)
        count = 1;

    for (unsigned int i = 0; i < count; ++i)
    {
        workers.emplace_back([this]()
            {
                workerLoop();
            });
    }
}

ThreadPool::~ThreadPool()
{
    {
        std::lock_guard<std::mutex> lock(mutex);
        stopping = true;
    }

    taskCV.notify_all();

    for (auto& worker : workers)
        worker.join();
}

void ThreadPool::submit(std::function<void()> task)
{
    {
        std::lock_guard<std::mutex> lock(mutex);
        tasks.push(std::move(task));
        ++pending;
    }

    taskCV.notify_one();
}

void ThreadPool::wait()
{
    std::unique_lock<std::mutex> lock(mutex);

    doneCV.wait(lock, [this]()
        {
            return pending == 0;
        });
}

void ThreadPool::workerLoop()
{
    while (true)
    {
        std::function<void()> task;

        {
            std::unique_lock<std::mutex> lock(mutex);

            taskCV.wait(lock, [this]()
                {
                    return stopping || !tasks.empty();
                });

            if (stopping && tasks.empty())
                return;

            task = std::move(tasks.front());
            tasks.pop();
        }

        // Выполняем задачу без блокировки mutex.
        task();

        {
            std::lock_guard<std::mutex> lock(mutex);

            if (--pending == 0)
                doneCV.notify_all();
        }
    }
}