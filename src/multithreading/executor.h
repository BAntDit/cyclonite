//
// Created by anton on 9/13/25.
//

#ifndef CYCLONITE_MT_EXECUTOR_H
#define CYCLONITE_MT_EXECUTOR_H

#include "common.h"
#include "mpscDeque.h"
#include "taskPool.h"
#include "taskStealingDeque.h"

namespace cyclonite::multithreading {
class TaskManager;

class Executor
{
    friend class TaskManager;

public:
    Executor() = default;

    explicit Executor(TaskManager& taskManager);

    Executor(Executor const&) = delete;

    Executor(Executor&&) = default;

    ~Executor() = default;

    auto operator=(Executor const&) -> Executor& = delete;

    auto operator=(Executor&&) -> Executor& = default;

    void operator()();

    void run();

    void runOne();

    template<typename F>
        requires std::is_invocable_v<F>
    auto submitTask(F&& f) -> std::future<std::invoke_result_t<F>>;

    [[nodiscard]] auto ownerThreadId() const -> std::thread::id { return threadId_; }

    [[nodiscard]] auto canSubmit() const -> bool;

    [[nodiscard]] auto taskManager() const -> TaskManager const& { return *taskManager_; }

    [[nodiscard]] auto taskManager() -> TaskManager& { return *taskManager_; }

    static auto isInMainThread() -> bool;

    static auto isInRenderThread() -> bool;

    static auto threadExecutor() -> Executor&;

private:
    template<typename F>
        requires std::is_invocable_v<F>
    auto emplaceTaskForThisExecutor(F&& f) -> std::future<std::invoke_result_t<F>>;

    auto pendingTask() -> std::optional<Task>;

    [[nodiscard]] auto poolSC() const -> TaskPoolSC const& { return taskPoolSC_; }
    [[nodiscard]] auto poolSC() -> TaskPoolSC& { return taskPoolSC_; }

    [[nodiscard]] auto poolMC() const -> TaskPoolMC const& { return taskPoolMC_; }
    [[nodiscard]] auto poolMC() -> TaskPoolMC& { return taskPoolMC_; }

    [[nodiscard]] auto spmcQueue() const -> TaskStealingDeque<Task*> const& { return *spmcQueue_; }
    [[nodiscard]] auto spmcQueue() -> TaskStealingDeque<Task*>& { return *spmcQueue_; }

    [[nodiscard]] auto mpscQueue() const -> MpscDeque<Task*> const& { return *mpscQueue_; }
    [[nodiscard]] auto mpscQueue() -> MpscDeque<Task*>& { return *mpscQueue_; }

    [[nodiscard]] auto taskManager() const -> TaskManager const&;
    [[nodiscard]] auto taskManager() -> TaskManager&;

    void _setThreadExecutorPtr();
    void _resetThreadExecutorPtr();

private:
    std::thread::id threadId_;
    TaskManager* taskManager_;
    TaskPoolSC taskPoolSC_;
    TaskPoolMC taskPoolMC_;
    std::unique_ptr<TaskStealingDeque<Task*>> spmcQueue_;
    std::unique_ptr<MpscDeque<Task*>> mpscQueue_;
};

template<typename F>
    requires std::is_invocable_v<F>
auto Executor::emplaceTaskForThisExecutor(F&& f) -> std::future<std::invoke_result_t<F>>
{
    assert(canSubmit());

    using result_type_t = std::invoke_result_t<F>;

    auto* task = std::add_pointer_t<Task>{ nullptr };

    while ((task = poolMC().writeableTask(), task == nullptr))
        std::this_thread::yield();

    auto&& packedTask = std::packaged_task<result_type_t()>{ std::forward<F>(f) };
    auto future = packedTask.get_future();

    *task = Task{ std::move(packedTask) };

    while (!mpscQueue().tryEmplace(task))
        std::this_thread::yield();

    return future;
}

template<typename F>
    requires std::is_invocable_v<F>
auto Executor::submitTask(F&& f) -> std::future<std::invoke_result_t<F>>
{
    assert(canSubmit());

    using result_type_t = std::invoke_result_t<F>;

    auto* task = std::add_pointer_t<Task>{ nullptr };

    while ((task = poolSC().writeableTask(), task == nullptr))
        std::this_thread::yield();

    auto&& packedTask = std::packaged_task<result_type_t()>{ std::forward<F>(f) };
    auto future = packedTask.get_future();

    *task = Task{ std::move(packedTask) };

    // cycle waits space in deque if there is no one
    // it must happen hardly ever as well
    while (!spmcQueue().tryEmplace(task))
        std::this_thread::yield();

    return future;
}
}

#endif // CYCLONITE_MT_EXECUTOR_H
