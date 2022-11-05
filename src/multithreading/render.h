//
// Created by bantdit on 10/30/22.
//

#ifndef CYCLONITE_RENDER_H
#define CYCLONITE_RENDER_H

#include "lockFreeQueue.h"
#include "taskPool.h"
#include <future>

namespace cyclonite::multithreading {
class TaskManager;

class Render
{
    friend class TaskManager;

public:
    Render(TaskManager& taskManager, size_t taskPoolSize);

    Render(Render const&) = delete;

    Render(Render&&) = delete;

    ~Render() = default;

    auto operator=(Render const&) -> Render& = delete;

    auto operator=(Render &&) -> Render& = delete;

    void operator()();

    template<TaskFunctor F>
    auto submitTask(F&& f) -> std::future<std::result_of_t<F()>>;

    [[nodiscard]] auto taskManager() const -> TaskManager const& { return *taskManager_; }
    auto taskManager() -> TaskManager& { return *taskManager_; }

private:
    [[nodiscard]] auto pool() const -> TaskPool const& { return taskPool_; }
    auto pool() -> TaskPool& { return taskPool_; }

    [[nodiscard]] auto queue() const -> lock_free_mpmc_queue_t<Task*> const& { return taskQueue_; }
    auto queue() -> lock_free_mpmc_queue_t<Task*>& { return taskQueue_; }

    auto pendingTask() -> std::optional<Task>;

    static auto renderThread() -> Render&;

public:
    static auto isInRenderThread() -> bool;

private:
    TaskManager* taskManager_;
    TaskPool taskPool_;
    lock_free_mpmc_queue_t<Task*> taskQueue_;
};

template<TaskFunctor F>
auto Render::submitTask(F&& f) -> std::future<std::result_of_t<F()>>
{
    using result_type_t = std::result_of_t<F()>;

    Task* task = nullptr;
    while ((task = pool().writeableTask(), task == nullptr))
        std::this_thread::yield();

    auto&& packedTask = std::packaged_task<result_type_t()>{ std::forward<F>(f) };
    auto future = packedTask.get_future();

    *task = Task{ std::move(packedTask) };

    [[maybe_unused]] auto success = queue().bounded_push(task);
    assert(success);

    return future;
}
}

#endif // CYCLONITE_RENDER_H
