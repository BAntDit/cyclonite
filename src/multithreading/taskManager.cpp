
#include "taskManager.h"
#include "config.h"
#include <cassert>

namespace cyclonite::multithreading {
TaskManager::TaskManager(size_t threadPoolSize /*= std::max(std::thread::hardware_concurrency(), 1u)*/)
  : threadPool_{}
  , executorCount_{ threadPoolSize + 1 } // plus main thread
  , executors_{ std::make_unique_for_overwrite<Executor[]>(executorCount_) }
  , executorIndexToStealTask_{ 0 }
  , taskPoolForStrand_{ config_t::strand_queue_max_size_v }
  , strandDeque_{ nullptr }
  , alive_{ true }
#if !defined(DISABLE_THREAD_EXCEPTIONS_PROPAGATION)
  , exPropagationLock_{}
  , exceptions_{}
#endif
{
    threadPool_.reserve(threadPoolSize);

    assert(executorCount_ >= 2);
    for (auto i = size_t{ 0 }; i < executorCount_; i++) {
        auto purpose = (i == renderExecutorIndex) ? Purpose::Render : Purpose::General;
        new (&executors_[i]) Executor{ *this, purpose };
    }
    executors_[0]._setAsMainThreadExecutor();
}

TaskManager::~TaskManager()
{
    stop();
    executors_[0]._resetMainThreadExecutor();
}

void TaskManager::start()
{
    assert(Executor::isInMainThread());

    for (auto i = size_t{ 0 }; i < executorCount_; i++) {
        if (executors_[i].canSubmit()) {
            executors_[i]();
        } else {
            threadPool_.emplace_back([](Executor& executor) -> void { executor(); }, std::ref(executors_[i]));
        }
    }
}

void TaskManager::stop()
{
    alive_.store(false, std::memory_order_release);

    for (auto&& thread : threadPool_) {
        if (thread.joinable())
            thread.join();
    }
}

auto TaskManager::executorIndexToStealTask() -> size_t
{
    return executorIndexToStealTask_.fetch_add(1, std::memory_order_acq_rel) % executorCount_;
}

#if !defined(DISABLE_THREAD_EXCEPTIONS_PROPAGATION)
auto TaskManager::getLastException() -> std::exception_ptr
{
    auto lock = std::lock_guard<core::SpinLock>{ exPropagationLock_ };

    auto ex = std::exception_ptr{};

    if (!exceptions_.empty()) {
        ex = exceptions_.back();
        exceptions_.pop_back();
    }

    return ex;
}

void TaskManager::propagateException(std::exception_ptr const& exception)
{
    auto lock = std::lock_guard<core::SpinLock>{ exPropagationLock_ };
    exceptions_.push_back(exception);
}
#endif
}
