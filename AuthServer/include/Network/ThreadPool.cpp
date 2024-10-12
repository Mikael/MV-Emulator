#include "ThreadPool.h"

// ThreadPool constructor
ThreadPool::ThreadPool(size_t numThreads) : stop(false)
{
    for (size_t i = 0; i < numThreads; ++i)
    {
        workers.emplace_back([this]
        {
            for (;;)
            {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(this->queueMutex);
                    this->condition.wait(lock, [this] { return this->stop || !this->tasks.empty(); });
                    if (this->stop && this->tasks.empty()) return;
                    task = std::move(this->tasks.front());
                    this->tasks.pop();
                }
                task();
            }
        });
    }
}

// ThreadPool destructor
ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        stop = true;
    }
    condition.notify_all();
    for (std::thread& worker : workers) worker.join();
}

// Enqueue a task in the thread pool
template<class F>
void ThreadPool::enqueue(F&& f)
{
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        tasks.emplace(std::forward<F>(f));
    }
    condition.notify_one();
}

// Explicitly instantiate the template for common types
template void ThreadPool::enqueue<std::function<void()>>(std::function<void()> &&);

