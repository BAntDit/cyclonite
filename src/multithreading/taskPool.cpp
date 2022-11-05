//
// Created by bantdit on 10/31/22.
//

#include "taskPool.h"
#include <random>

namespace cyclonite::multithreading {
TaskPool::TaskPool(size_t size)
  : size_{ size }
  , tasks_{ std::make_unique_for_overwrite<Task[]>(size) }
{}

auto TaskPool::writeableTask() -> Task*
{
    Task* taskPtr = nullptr;
    for (auto i = size_t{ 0 }; i < size_; i++) {
        auto& task = tasks_[i];

        if (!task.pending()) {
            taskPtr = &task;
            break;
        }
    }

    return taskPtr;
}
}