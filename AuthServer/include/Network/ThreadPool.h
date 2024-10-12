#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <functional>
#include <thread>
#include <mutex>
#include <vector>
#include <queue>
#include <condition_variable>

class ThreadPool
{
public:
    ThreadPool(size_t numThreads);
    ~ThreadPool();

    template<class F>
    void enqueue(F&& f);

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;

    std::mutex queueMutex;
    std::condition_variable condition;
    bool stop;
};

#endif // THREADPOOL_H
