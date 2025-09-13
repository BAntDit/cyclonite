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

    Executor(TaskManager& taskManager, size_t workStealingDequeSize, size_t directDequeSize);

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

    template<typename F>
        requires std::is_invocable_v<F>
    auto directTask(std::thread::id, F&& f) -> std::future<std::invoke_result_t<F>>;

    [[nodiscard]] auto ownerThreadId() const -> std::thread::id { return threadId_; }

    [[nodiscard]] auto canSubmit() const -> bool;

    [[nodiscard]] auto taskManager() const -> TaskManager const& { return *taskManager_; }

    [[nodiscard]] auto taskManager() -> TaskManager& { return *taskManager_; }

    static auto isInMainThread() -> bool;

    static auto isInRenderThread() -> bool;

    static auto threadExecutor() -> Executor&;

private:
    std::thread::id threadId_;
    TaskManager* taskManager_;
    TaskPool taskPool_;
    TaskPool directTaskPool_;
    std::unique_ptr<TaskStealingDeque<Task*>> spmcQueue_;
    std::unique_ptr<MpscDeque<Task*>> mpscQueue_;
};
}

#endif // CYCLONITE_MT_EXECUTOR_H
