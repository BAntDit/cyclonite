//
// Created by anton on 9/13/25.
//

#ifndef CYCLONITE_MT_EXECUTOR_H
#define CYCLONITE_MT_EXECUTOR_H

#include "mpscDeque.h"
#include "multithreading/common.h"
#include "taskPool.h"
#include "taskStealingDeque.h"

namespace cyclonite::multithreading {
class TaskManager;

class Executor
{
    friend class TaskManager;

public:
    Executor() = default;

    Executor(TaskManager& taskManager, size_t executorIndex);

    Executor(Executor const&) = delete;

    Executor(Executor&&) = default;

    ~Executor() = default;

    auto operator=(Executor const&) -> Executor& = delete;

    auto operator=(Executor&&) -> Executor& = default;

    void operator()(PurposeBits purpose = PurposeBits{ Purpose::General });

    void run(PurposeBits purpose = PurposeBits{ Purpose::General });

    auto runOne() -> bool;

    template<typename F>
        requires std::is_invocable_v<F>
    auto submitTask(F&& f, Purpose purpose = Purpose::General) -> std::future<std::invoke_result_t<F>>;

    [[nodiscard]] auto ownerThreadId() const -> std::thread::id { return threadId_; }

    [[nodiscard]] auto canSubmit() const -> bool;

    [[nodiscard]] auto matchesPurpose(Purpose taskPurpose) const -> bool;

    [[nodiscard]] auto taskManager() const -> TaskManager const& { return *taskManager_; }
    [[nodiscard]] auto taskManager() -> TaskManager& { return *taskManager_; }

    static auto isInMainThread() -> bool;

    static auto isInRenderThread() -> bool;

    static auto isInTransferThread() -> bool;

    static auto isInComputeThread() -> bool;

    static auto threadExecutor() -> Executor&;

private:
    template<typename F>
        requires std::is_invocable_v<F>
    auto submitTaskSPMC(F&& f) -> std::future<std::invoke_result_t<F>>;

    template<typename F>
        requires std::is_invocable_v<F>
    auto submitTaskMPSC(F&& f) -> std::future<std::invoke_result_t<F>>;

    template<typename F>
        requires std::is_invocable_v<F>
    auto executeInPlace(F&& f) -> std::future<std::invoke_result_t<F>>;

    auto pendingTask() -> std::optional<Task>;

    auto renderExecutor() -> Executor&;

    auto transferExecutor() -> Executor&;

    auto computeExecutor() -> Executor&;

    void notifyNewTask();

    [[nodiscard]] auto poolSC() const -> TaskPoolSC const& { return taskPoolSC_; }
    [[nodiscard]] auto poolSC() -> TaskPoolSC& { return taskPoolSC_; }

    [[nodiscard]] auto poolMC() const -> TaskPoolMC const& { return taskPoolMC_; }
    [[nodiscard]] auto poolMC() -> TaskPoolMC& { return taskPoolMC_; }

    [[nodiscard]] auto spmcQueue() const -> TaskStealingDeque<task_ptr_t> const& { return *spmcQueue_; }
    [[nodiscard]] auto spmcQueue() -> TaskStealingDeque<task_ptr_t>& { return *spmcQueue_; }

    [[nodiscard]] auto mpscQueue() const -> MpscDeque<task_ptr_t> const& { return *mpscQueue_; }
    [[nodiscard]] auto mpscQueue() -> MpscDeque<task_ptr_t>& { return *mpscQueue_; }

    void _setThreadExecutorPtr(PurposeBits purpose);
    void _resetThreadExecutorPtr();

    void _setAsMainThreadExecutor();
    void _resetMainThreadExecutor();

private:
    size_t executorIndex_;
    std::thread::id threadId_;
    TaskManager* taskManager_;
    TaskPoolSC taskPoolSC_;
    TaskPoolMC taskPoolMC_;
    std::unique_ptr<TaskStealingDeque<task_ptr_t>> spmcQueue_;
    std::unique_ptr<MpscDeque<task_ptr_t>> mpscQueue_;
};

template<typename F>
    requires std::is_invocable_v<F>
auto Executor::submitTask(F&& f, Purpose purpose /*= Purpose::General*/) -> std::future<std::invoke_result_t<F>>
{
    auto future = std::future<std::invoke_result_t<F>>{};

    if (purpose == Purpose::Render) {
        if (isInRenderThread()) {
            future = executeInPlace(std::forward<F>(f));
        } else {
            future = renderExecutor().submitTaskMPSC(std::forward<F>(f));
        }
    } else if (purpose == Purpose::Transfer) {
        if (isInTransferThread()) {
            future = executeInPlace(std::forward<F>(f));
        } else {
            future = transferExecutor().submitTaskMPSC(std::forward<F>(f));
        }
    } else if (purpose == Purpose::Compute) {
        if (isInComputeThread()) {
            future = executeInPlace(std::forward<F>(f));
        } else {
            future = computeExecutor().submitTaskMPSC(std::forward<F>(f));
        }
    } else {
        assert(purpose == Purpose::General);
        future = submitTaskSPMC(std::forward<F>(f));
    }

    notifyNewTask();

    return future;
}

template<typename F>
    requires std::is_invocable_v<F>
auto Executor::submitTaskMPSC(F&& f) -> std::future<std::invoke_result_t<F>>
{
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
auto Executor::submitTaskSPMC(F&& f) -> std::future<std::invoke_result_t<F>>
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

template<typename F>
    requires std::is_invocable_v<F>
auto Executor::executeInPlace(F&& f) -> std::future<std::invoke_result_t<F>>
{
    using result_type_t = std::invoke_result_t<F>;

    auto&& packedTask = std::packaged_task<result_type_t()>{ std::forward<F>(f) };
    auto future = packedTask.get_future();

    auto&& task = Task{ std::move(packedTask) };
    task();

    return future;
}
}

#endif // CYCLONITE_MT_EXECUTOR_H
