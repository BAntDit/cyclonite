//
// Created by anton on 9/13/25.
//

#ifndef CYCLONITE_TASKMANAGER_H
#define CYCLONITE_TASKMANAGER_H

#include "multithreading/common.h"
#include "core/spinLock.h"
#include "executor.h"
#include "strandDeque.h"
#include "taskPool.h"
#include <condition_variable>
#include <vector>

namespace cyclonite::multithreading {
class TaskManager
{
    friend class Executor;

    static size_t renderExecutorIndex;
    static size_t computeExecutorIndex;
    static size_t transferExecutorIndex;

public:
    TaskManager(bool dedicatedTransferRequired,
                bool dedicatedComputeRequired,
                size_t threadPoolSize = std::max(std::thread::hardware_concurrency(), 1u));

    TaskManager(TaskManager const&) = delete;

    TaskManager(TaskManager&&) = delete;

    ~TaskManager();

    auto operator=(TaskManager const&) -> TaskManager& = delete;

    auto operator=(TaskManager&&) -> TaskManager& = delete;

    void start();

    void stop();

    [[nodiscard]] auto keepAlive() const -> bool { return alive_.load(std::memory_order_relaxed); }

    [[nodiscard]] auto executorCount() const -> size_t { return executorCount_; }

    auto getLastException() -> std::exception_ptr;

    template<typename F>
        requires std::is_invocable_v<F>
    static auto submitTask(F&& f, Purpose purpose = Purpose::General) -> std::future<std::invoke_result_t<F>>;

    template<typename F>
        requires std::is_invocable_v<F>
    static auto strandTask(F&& f) -> std::future<std::invoke_result_t<F>>;

    [[nodiscard]] auto getExecutorPurposeBits(Purpose purpose) const -> PurposeBits;

private:
    [[nodiscard]] auto executors() const -> std::unique_ptr<Executor[]> const& { return executors_; }
    [[nodiscard]] auto executors() -> std::unique_ptr<Executor[]>& { return executors_; }

    [[nodiscard]] auto taskPool() const -> TaskPoolMC const& { return taskPoolForStrand_; }
    [[nodiscard]] auto taskPool() -> TaskPoolMC& { return taskPoolForStrand_; }

    [[nodiscard]] auto strandQueue() const -> StrandDeque const& { return *strandDeque_; }
    [[nodiscard]] auto strandQueue() -> StrandDeque& { return *strandDeque_; }

    [[nodiscard]] auto executorIndexToStealTask() -> size_t;

    void waitForTasks();
    void notifyNewTask();
    void notifyNoTasks();

#if !defined(DISABLE_THREAD_EXCEPTIONS_PROPAGATION)
    void propagateException(std::exception_ptr const& exception);
#endif

private:
    std::vector<std::thread> threadPool_;

    size_t executorCount_;
    std::unique_ptr<Executor[]> executors_;
    std::unique_ptr<PurposeBits[]> executorPurposes_;
    std::atomic<size_t> executorIndexToStealTask_;

    TaskPoolMC taskPoolForStrand_;
    std::unique_ptr<StrandDeque> strandDeque_;

    std::atomic<bool> alive_;

    bool noTasks_;
    std::condition_variable_any noTaskCv_;
    core::SpinLock noTaskLock_;

#if !defined(DISABLE_THREAD_EXCEPTIONS_PROPAGATION)
    core::SpinLock exPropagationLock_;
    std::vector<std::exception_ptr> exceptions_;
#endif
};

template<typename F>
    requires std::is_invocable_v<F>
auto TaskManager::submitTask(F&& f, Purpose purpose /* = Purpose::General*/) -> std::future<std::invoke_result_t<F>>
{
    return Executor::threadExecutor().submitTask(std::forward<F>(f), purpose);
}

template<typename F>
    requires std::is_invocable_v<F>
auto TaskManager::strandTask(F&& f) -> std::future<std::invoke_result_t<F>>
{
    using result_type_t = std::invoke_result_t<F>;

    auto* task = std::add_pointer_t<Task>{ nullptr };

    auto& taskManager = Executor::threadExecutor().taskManager();

    while ((task = taskManager.taskPool().writeableTask(), task == nullptr))
        std::this_thread::yield();

    auto&& packedTask = std::packaged_task<result_type_t()>{ std::forward<F>(f) };
    auto future = packedTask.get_future();

    *task = Task{ std::move(packedTask) };

    // cycle waits space in deque if there is no one
    // it must happen hardly ever as well
    while (taskManager.strandQueue().tryEmplace(task))
        std::this_thread::yield();

    taskManager.notifyNewTask();

    return future;
}
}

#endif // CYCLONITE_TASKMANAGER_H
