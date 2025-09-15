//
// Created by anton on 9/13/25.
//

#include "executor.h"
#include "config.h"
#include "taskManager.h"

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

void Executor::runOne() {}
}
