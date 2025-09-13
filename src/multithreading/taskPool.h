//
// Created by anton on 9/13/25.
//

#ifndef CYCLONITE_MT_TASKPOOL_H
#define CYCLONITE_MT_TASKPOOL_H

#include "task.h"

namespace cyclonite::multithreading {
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

    auto writeableTask() -> Task*;

private:
    size_t size_;
    std::unique_ptr<Task[]> tasks_;
};
}

#endif // CYCLONITE_MT_TASKPOOL_H
