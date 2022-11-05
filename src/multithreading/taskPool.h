//
// Created by bantdit on 10/30/22.
//

#ifndef CYCLONITE_TASKPOOL_H
#define CYCLONITE_TASKPOOL_H

#include "task.h"
#include <cstddef>
#include <memory>

namespace cyclonite::multithreading {
class TaskManager;

class TaskPool
{
public:
    TaskPool() = default;

    explicit TaskPool(size_t size);

    TaskPool(TaskPool const&) = delete;

    TaskPool(TaskPool&&) = default;

    ~TaskPool() = default;

    auto operator=(TaskPool const&) -> TaskPool& = delete;

    auto operator=(TaskPool &&) -> TaskPool& = default;

    auto writeableTask() -> Task*;

private:
    size_t size_;
    std::unique_ptr<Task[]> tasks_;
};
}

#endif // CYCLONITE_TASKPOOL_H
