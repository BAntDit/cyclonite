//
// Created by anton on 9/13/25.
//

#include "executor.h"
#include "config.h"
#include "taskManager.h"

#if !defined(DISABLE_THREAD_EXCEPTIONS_PROPAGATION)
#define BEGIN_EXCEPTION_PROPAGATION() try {

#define END_EXCEPTION_PROPAGATION()                                                                                    \
    }                                                                                                                  \
    catch (...)                                                                                                        \
    {                                                                                                                  \
        taskManager().propagateException(std::current_exception());                                                    \
    }
#else
#define BEGIN_EXCEPTION_PROPAGATION()
#define END_EXCEPTION_PROPAGATION()
#endif

namespace cyclonite::multithreading {
namespace {
thread_local Executor* _mainThreadExecutor = nullptr;
thread_local Executor* _threadExecutor = nullptr;
thread_local PurposeBits _executorPurposeBits = PurposeBits{};
}

/*static*/ auto Executor::isInMainThread() -> bool
{
    return _mainThreadExecutor != nullptr;
}

/*static*/ auto Executor::isInRenderThread() -> bool
{
    return _executorPurposeBits.test(Purpose::Render);
}

/*static*/ auto Executor::isInTransferThread() -> bool
{
    return _executorPurposeBits.test(Purpose::Transfer);
}

/*static*/ auto Executor::isInComputeThread() -> bool
{
    return _executorPurposeBits.test(Purpose::Compute);
}

/*static*/ auto Executor::threadExecutor() -> Executor&
{
    assert(_threadExecutor != nullptr);
    return *_threadExecutor;
}

Executor::Executor(TaskManager& taskManager, size_t executorIndex)
  : executorIndex_{ executorIndex }
  , threadId_{}
  , taskManager_{ &taskManager }
  , taskPoolSC_{ config_t::spmc_queue_max_size_v } // one producer consumes from pool
  , taskPoolMC_{ config_t::mpsc_queue_max_size_v } // many producers can consume from pool
  , spmcQueue_{ nullptr }
  , mpscQueue_{ nullptr }
{
    spmcQueue_ = std::make_unique<TaskStealingDeque<Task*>>(config_t::spmc_queue_max_size_v);
    mpscQueue_ = std::make_unique<MpscDeque<Task*>>(config_t::mpsc_queue_max_size_v);
}

auto Executor::canSubmit() const -> bool
{
    return (_threadExecutor != nullptr) && threadId_ == std::this_thread::get_id();
}

auto Executor::matchesPurpose(Purpose taskPurpose) const -> bool
{
    return taskManager_->executorPurposes_[executorIndex_].test(taskPurpose);
}

void Executor::_setThreadExecutorPtr(PurposeBits purpose)
{
    assert(_threadExecutor == nullptr);
    _threadExecutor = this;

    _executorPurposeBits = purpose;

    threadId_ = std::this_thread::get_id();
}

void Executor::_resetThreadExecutorPtr()
{
    _executorPurposeBits = PurposeBits{};
    _threadExecutor = nullptr;
    threadId_ = std::thread::id{};
}

void Executor::_setAsMainThreadExecutor()
{
    assert(_mainThreadExecutor == nullptr);
    _mainThreadExecutor = this;
    _threadExecutor = this;

    threadId_ = std::this_thread::get_id();
}

void Executor::_resetMainThreadExecutor()
{
    _mainThreadExecutor = nullptr;
    _threadExecutor = nullptr;

    threadId_ = std::thread::id{};
}

auto Executor::pendingTask() -> std::optional<Task>
{
    auto task = std::optional<Task>{ std::nullopt };

    if (auto directedTask = mpscQueue().tryPop()) {
        task = std::move(*directedTask.value());
    } else if (auto ownTask = spmcQueue().tryPop()) {
        task = std::move(*ownTask.value());
    } else if (auto strandTask = taskManager().strandQueue().tryPop()) {
        task = std::move(*strandTask.value());
    } else {
        for (auto i = size_t{ 0 }, count = taskManager().executorCount(); i < count; i++) {
            auto executorIndex = taskManager().executorIndexToStealTask();
            auto& executors = taskManager().executors();
            auto& executor = executors[executorIndex];

            if (executor.ownerThreadId() == std::this_thread::get_id())
                continue;

            if (auto stolenTask = executor.spmcQueue().trySteal()) {
                task = std::move(*stolenTask.value());
                break;
            }
        }
    }

    return task;
}

auto Executor::runOne() -> bool
{
    if (auto task = pendingTask()) {
        task.value()();
        return true;
    }

    return false;
}

void Executor::run(PurposeBits purpose /* = PurposeBits{ Purpose::General }*/)
{
    if (isInMainThread())
        return;

    _setThreadExecutorPtr(purpose);

    BEGIN_EXCEPTION_PROPAGATION();

    while (taskManager().keepAlive()) {
        taskManager().waitForTasks();

        while (runOne()) {
        }

        taskManager().notifyNoTasks();
    }

    END_EXCEPTION_PROPAGATION();

    _resetThreadExecutorPtr();
}

void Executor::operator()(PurposeBits purpose /*= PurposeBits{ Purpose::General }*/)
{
    run(purpose);
}

auto Executor::renderExecutor() -> Executor&
{
    return taskManager().executors()[TaskManager::renderExecutorIndex];
}

auto Executor::transferExecutor() -> Executor&
{
    return taskManager().executors()[TaskManager::transferExecutorIndex];
}

auto Executor::computeExecutor() -> Executor&
{
    return taskManager().executors()[TaskManager::computeExecutorIndex];
}

void Executor::notifyNewTask()
{
    taskManager().notifyNewTask();
}
}
