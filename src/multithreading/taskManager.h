//
// Created by anton on 9/13/25.
//

#ifndef CYCLONITE_TASKMANAGER_H
#define CYCLONITE_TASKMANAGER_H

#include "common.h"
#include "taskPool.h"
#include "strandDeque.h"
#include "executor.h"
#include "core/spinLock.h"

namespace cyclonite::multithreading {
class TaskManager
{
    friend class Executor;

public:
    [[nodiscard]] auto keepAlive() const -> bool { return alive_.load(std::memory_order_relaxed); }
    
    [[nodiscard]] auto executorCount() const -> size_t { return executorCount_; }

#if !defined(DISABLE_THREAD_EXCEPTIONS_PROPAGATION)
    auto getLastException() -> std::exception_ptr;
#endif

private:
    [[nodiscard]] auto executors() const -> std::unique_ptr<Executor[]> const& { return executors_; }
    [[nodiscard]] auto executors() -> std::unique_ptr<Executor[]>& { return executors_; }

    [[nodiscard]] auto taskPool() const -> TaskPoolMC const& { return taskPoolForStrand_; }
    [[nodiscard]] auto taskPool() -> TaskPoolMC& { return taskPoolForStrand_; }

    [[nodiscard]] auto strandQueue() const -> StrandDeque const& { return *strandDeque_; }
    [[nodiscard]] auto strandQueue() -> StrandDeque& { return *strandDeque_; }

    [[nodiscard]] auto executorIndexToStealTask() -> size_t;

#if !defined(DISABLE_THREAD_EXCEPTIONS_PROPAGATION)
    void propagateException(std::exception_ptr const& exception);
#endif

private:
    std::vector<std::thread> threadPool_;

    size_t executorCount_;
    std::unique_ptr<Executor[]> executors_;
    std::atomic<size_t> executorIndexToStealTask_;

    TaskPoolMC taskPoolForStrand_;
    std::unique_ptr<StrandDeque> strandDeque_;

    std::atomic<bool> alive_;

#if !defined(DISABLE_THREAD_EXCEPTIONS_PROPAGATION)
    core::SpinLock exPropagationLock_;
    std::vector<std::exception_ptr> exceptions_;
#endif
};
}

#endif // CYCLONITE_TASKMANAGER_H
