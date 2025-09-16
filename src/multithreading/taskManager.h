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

private:
    [[nodiscard]] auto taskPool() const -> TaskPoolMC const& { return taskPoolForStrand_; }
    [[nodiscard]] auto taskPool() -> TaskPoolMC& { return taskPoolForStrand_; }

    [[nodiscard]] auto strandQueue() const -> StrandDeque const& { return *strandDeque_; }
    [[nodiscard]] auto strandQueue() -> StrandDeque& { return *strandDeque_; }

private:
    std::vector<std::thread> threadPool_;

    size_t executorCount_;
    std::unique_ptr<Executor[]> executors_;

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
