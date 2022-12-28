//
// Created by bantdit on 10/29/22.
//

#include "worker.h"
#include "taskManager.h"
#include <random>

namespace cyclonite::multithreading {
static thread_local Worker* _threadWorker = nullptr;
auto Worker::threadWorker() -> Worker&
{
    assert(_threadWorker);
    return *_threadWorker;
}

auto Worker::isInWorkerThread() -> bool
{
    return _threadWorker;
}

Worker::Worker(TaskManager& taskManager, size_t size)
  : threadId_{}
  , taskManager_{ &taskManager }
  , taskPool_{ size }
  , workerQueue_{ nullptr }
{
    workerQueue_ = std::make_unique<lock_free_spmc_queue_t<Task*>>(size);
}

auto Worker::pendingTask() -> std::optional<Task>
{
    auto task = std::optional<Task>{ std::nullopt };

    if (auto taskPointer = queue().popBottom()) {
        task = std::move(*taskPointer.value());
    } else {
        auto workerCount = taskManager().workerCount();
        auto workerIndex = randomWorkerIndex(workerCount);
        auto& workers = taskManager().workers();
        auto& worker = workers[workerIndex];

        if (auto stolenTaskPtr = worker.queue().steal()) {
            task = std::move(*stolenTaskPtr.value());
        }
    }

    return task;
}

void Worker::operator()()
{
    _setThreadWorkerPtr();

    threadId_ = std::this_thread::get_id();

    try {
        while (taskManager().keepAlive()) {
            if (auto task = pendingTask()) {
                task.value()();
            } else {
                std::this_thread::yield();
            }
        }
    }
    catch(...) {
        taskManager().propagateException(std::current_exception());
    }

    _resetThreadWorkerPtr();
}

auto Worker::randomWorkerIndex(size_t count) -> size_t
{
    assert(count > 0);

    static thread_local auto rd = std::random_device();
    static thread_local auto generator = std::mt19937{ rd() };

    std::uniform_int_distribution<int> distribution(0, static_cast<int>(count) - 1);

    return static_cast<size_t>(distribution(generator));
}

auto Worker::canSubmit() const -> bool
{
    return _threadWorker && threadId_ == std::this_thread::get_id();
}

void Worker::_setThreadWorkerPtr()
{
    assert(_threadWorker == nullptr);
    _threadWorker = this;
}

void Worker::_resetThreadWorkerPtr()
{
    _threadWorker = nullptr;
}
}