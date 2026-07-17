//
// Created by anton on 9/13/25.
//

#ifndef CYCLONITE_MT_TASKPOOL_H
#define CYCLONITE_MT_TASKPOOL_H

#include "task.h"

namespace cyclonite::multithreading {
namespace internal {
class TaskPool
{
public:
    TaskPool() = default;

    explicit TaskPool(size_t size);

    TaskPool(TaskPool const&) = delete;

    TaskPool(TaskPool&&) = default;

    ~TaskPool() = default;

    auto operator=(TaskPool const&) -> TaskPool& = delete;

    auto operator=(TaskPool&&) -> TaskPool& = default;

protected:
    size_t size_;
    std::unique_ptr<Task[]> tasks_;
};
}

class TaskPoolSC : public internal::TaskPool
{
public:
    TaskPoolSC() = default;

    explicit TaskPoolSC(size_t size);

    auto writeableTask() -> Task*;
};

class TaskPoolMC : public internal::TaskPool
{
public:
    TaskPoolMC() = default;

    explicit TaskPoolMC(size_t size);

    auto writeableTask() -> Task*;
};
}

#endif // CYCLONITE_MT_TASKPOOL_H
