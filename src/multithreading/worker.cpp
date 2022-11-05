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

    if (auto task_pointer = queue().popBottom()) {
        task = std::move(*task_pointer.value());
    } else {
        auto& workers = taskManager().workers();
        auto& worker = workers[randomWorkerIndex(workers.size())];

        if (auto stolen_task_ptr = worker.queue().steal()) {
            task = std::move(*stolen_task_ptr.value());
        }
    }

    return task;
}

void Worker::operator()()
{
    assert(_threadWorker == nullptr);
    _threadWorker = this;

    threadId_ = std::this_thread::get_id();

    while (taskManager().keepAlive()) {
        if (auto task = pendingTask()) {
            task.value()();
        } else {
            std::this_thread::yield();
        }
    }
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
}