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
    catch (...) { taskManager().propagateException(std::current_exception()); }
#else
#define BEGIN_EXCEPTION_PROPAGATION()
#define END_EXCEPTION_PROPAGATION()
#endif

namespace cyclonite::multithreading {
namespace {
thread_local Executor* _mainThreadExecutor = nullptr;
thread_local Executor* _threadExecutor = nullptr;
}

/*static*/ auto Executor::isInMainThread() -> bool
{
    return _mainThreadExecutor != nullptr;
}

/*static*/ auto Executor::threadExecutor() -> Executor&
{
    assert(_threadExecutor != nullptr);
    return *_threadExecutor;
}

Executor::Executor(TaskManager& taskManager)
  : threadId_{}
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

auto Executor::taskManager() const -> TaskManager const&
{
    return *taskManager_;
}

auto Executor::taskManager() -> TaskManager&
{
    return *taskManager_;
}

void Executor::_setThreadExecutorPtr()
{
    assert(_threadExecutor == nullptr);
    _threadExecutor = this;

    threadId_ = std::this_thread::get_id();
}

void Executor::_resetThreadExecutorPtr()
{
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
            auto executorIndex = TaskManager().executorIndexToStealTask();
            auto& executors = TaskManager().executors();
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

void Executor::runOne()
{
    if (auto task = pendingTask()) {
        task.value()();
    } else {
        std::this_thread::yield();
    }
}

void Executor::run()
{
    if (isInMainThread())
        return;

    _setThreadExecutorPtr();

    BEGIN_EXCEPTION_PROPAGATION();

    while (taskManager().keepAlive()) {
        runOne();
    }

    END_EXCEPTION_PROPAGATION();

    _resetThreadExecutorPtr();
}

void Executor::operator()()
{
    run();
}
}
