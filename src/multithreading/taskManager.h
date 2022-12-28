//
// Created by bantdit on 1/19/19.
//

#ifndef CYCLONITE_TASKMANAGER_H
#define CYCLONITE_TASKMANAGER_H

#include "render.h"
#include "worker.h"
#include <thread>
#include <type_traits>
#include <vector>

namespace cyclonite::multithreading {

/**
 * Task manager class
 */
class TaskManager
{
    friend class Worker;
    friend class Render;

public:
    explicit TaskManager(size_t workerCount = std::max(std::thread::hardware_concurrency(), 2u) - 1u);

    TaskManager(TaskManager const&) = delete;

    TaskManager(TaskManager&&) = delete;

    ~TaskManager();

    auto operator=(TaskManager const&) -> TaskManager& = delete;

    auto operator=(TaskManager &&) -> TaskManager& = delete;

    [[nodiscard]] auto keepAlive() const -> bool { return alive_.load(std::memory_order_relaxed); }

    [[nodiscard]] auto workerCount() const -> uint32_t { return workerCount_; }

    template<TaskFunctor F>
    auto start(F&& f) -> std::future<std::result_of_t<F()>>;

    void stop();

    template<TaskFunctor F>
    auto submitRenderTask(F&& f) -> std::future<std::result_of_t<F()>>;

    template<TaskFunctor F>
    auto submitTask(F&& f) -> std::future<std::result_of_t<F()>>;

    auto getLastException() -> std::exception_ptr;

private:
    void propagateException(std::exception_ptr const& exception);

    [[nodiscard]] auto workers() const -> std::unique_ptr<Worker[]> const& { return workers_; }

    auto workers() -> std::unique_ptr<Worker[]>& { return workers_; }

private:
    std::vector<std::exception_ptr> exceptions_;
    std::vector<std::thread> threadPool_;
    std::unique_ptr<Worker[]> workers_;
    size_t workerCount_;
    Render render_;
    std::mutex exceptionMutex_;
    std::atomic<bool> alive_;
};

template<TaskFunctor F>
auto TaskManager::submitRenderTask(F&& f) -> std::future<std::result_of_t<F()>>
{
    return render_.submitTask(std::forward<F>(f));
}

template<TaskFunctor F>
auto TaskManager::start(F&& f) -> std::future<std::result_of_t<F()>>
{
    auto const firstWorkerThread = size_t{ 1 };

    for (auto i = firstWorkerThread; i < workerCount_; i++)
        threadPool_.emplace_back([](Worker& worker) -> void { worker(); }, std::ref(workers_[i]));

    return workers_[0](std::forward<F>(f));
}

template<TaskFunctor F>
auto TaskManager::submitTask(F&& f) -> std::future<std::result_of_t<F()>>
{
    return workers_[0].submitTask(std::forward<F>(f));
}
}

#endif // CYCLONITE_TASKMANAGER_H
