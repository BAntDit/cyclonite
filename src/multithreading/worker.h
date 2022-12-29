//
// Created by bantdit on 10/29/22.
//

#ifndef CYCLONITE_WORKER_H
#define CYCLONITE_WORKER_H

#include "lockFreeQueue.h"
#include "task.h"
#include "taskPool.h"
#include <future>
#include <thread>
#include <type_traits>
#include <vector>

namespace cyclonite::multithreading {
class TaskManager;

class Worker
{
    friend class TaskManager;

public:
    Worker() = default;

    Worker(TaskManager& taskManager, size_t size);

    Worker(Worker const&) = delete;

    Worker(Worker&&) noexcept = default;

    ~Worker() = default;

    auto operator=(Worker const&) -> Worker& = delete;

    auto operator=(Worker&&) noexcept -> Worker& = default;

    void operator()();

    template<TaskFunctor F>
    auto operator()(F&& f) -> std::future<std::result_of_t<F()>>;

    template<TaskFunctor F>
    auto submitTask(F&& f) -> std::future<std::result_of_t<F()>>;

    template<TaskFunctor F>
    auto submitRenderTask(F&& f) -> std::future<std::result_of_t<F()>>;

    [[nodiscard]] auto threadId() const -> std::thread::id { return threadId_; }

    [[nodiscard]] auto canSubmit() const -> bool;

    [[nodiscard]] auto taskManager() const -> TaskManager const& { return *taskManager_; }
    auto taskManager() -> TaskManager& { return *taskManager_; }

    static auto isInMainThread() -> bool;

    static auto isInWorkerThread() -> bool;

    static auto threadWorker() -> Worker&;

private:
    [[nodiscard]] auto pool() const -> TaskPool const& { return taskPool_; }
    auto pool() -> TaskPool& { return taskPool_; }

    [[nodiscard]] auto queue() const -> lock_free_spmc_queue_t<Task*> const& { return *workerQueue_; }
    auto queue() -> lock_free_spmc_queue_t<Task*>& { return *workerQueue_; }

    [[nodiscard]] auto renderQueue() const -> lock_free_spmc_queue_t<Task*> const& { return *renderQueue_; }
    auto renderQueue() -> lock_free_spmc_queue_t<Task*>& { return *renderQueue_; }

    auto pendingTask() -> std::optional<Task>;

    void _setThreadWorkerPtr();

    void _resetThreadWorkerPtr();

    void _setAsMainThreadWorker();

private:
    std::thread::id threadId_;
    TaskManager* taskManager_;
    TaskPool taskPool_;
    std::unique_ptr<lock_free_spmc_queue_t<Task*>> workerQueue_;
    std::unique_ptr<lock_free_spmc_queue_t<Task*>> renderQueue_;
};

template<TaskFunctor F>
auto Worker::submitTask(F&& f) -> std::future<std::result_of_t<F()>>
{
    assert(canSubmit());

    using result_type_t = std::result_of_t<F()>;

    Task* task = nullptr;
    while ((task = pool().writeableTask(), task == nullptr))
        std::this_thread::yield();

    auto&& packedTask = std::packaged_task<result_type_t()>{ std::forward<F>(f) };
    auto future = packedTask.get_future();

    *task = Task{ std::move(packedTask) };

    queue().emplaceBottom(task);

    return future;
}

template<TaskFunctor F>
auto Worker::submitRenderTask(F&& f) -> std::future<std::result_of_t<F()>>
{
    assert(canSubmit());

    using result_type_t = std::result_of_t<F()>;

    Task* task = nullptr;
    while ((task = pool().writeableTask(), task == nullptr))
        std::this_thread::yield();

    auto&& packedTask = std::packaged_task<result_type_t()>{ std::forward<F>(f) };
    auto future = packedTask.get_future();

    *task = Task{ std::move(packedTask) };

    renderQueue().emplaceBottom(task);

    return future;
}

template<TaskFunctor F>
auto Worker::operator()(F&& f) -> std::future<std::result_of_t<F()>>
{
    _setThreadWorkerPtr();

    threadId_ = std::this_thread::get_id();

    using result_type_t = std::result_of_t<F()>;

    auto&& task = std::packaged_task<result_type_t()>{ std::forward<F>(f) };
    auto future = task.get_future();

    task();

    return future;
}
}

#endif // CYCLONITE_WORKER_H
