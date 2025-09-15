//
// Created by anton on 9/13/25.
//

#include "taskPool.h"
#include <utility>

namespace cyclonite::multithreading {
namespace internal {
TaskPool::TaskPool(size_t size)
  : size_{ size }
  , tasks_{ std::make_unique_for_overwrite<Task[]>(size) }
{
}
}

TaskPoolSC::TaskPoolSC(size_t size)
  : internal::TaskPool(size)
{
}

auto TaskPoolSC::writeableTask() -> Task*
{
    auto* taskPtr = std::add_pointer_t<Task>{ nullptr };

    for (auto i = size_t{ 0 }; i < size_; i++) {
        auto& task = tasks_[i];

        // it is safe, because each executor has own pool
        // so, false negative condition because of races is not possible
        if (!std::as_const(task).pending()) {
            taskPtr = &task;
            break;
        }
    }

    return taskPtr;
}

TaskPoolMC::TaskPoolMC(size_t size)
  : internal::TaskPool(size)
{
}

auto TaskPoolMC::writeableTask() -> Task*
{
    auto* taskPtr = std::add_pointer_t<Task>{ nullptr };

    for (auto i = size_t{ 0 }; i < size_; i++) {
        auto& task = tasks_[i];

        auto expected = false;
        auto desired = true;
        if (task.pending().compare_exchange_weak(
              expected, desired, std::memory_order_acq_rel, std::memory_order_relaxed)) {
            taskPtr = &task;
            break;
        }
    }

    return taskPtr;
}
}
