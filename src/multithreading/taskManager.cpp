
#include "taskManager.h"
#include "config.h"
#include <cassert>

namespace cyclonite::multithreading {
size_t TaskManager::renderExecutorIndex = std::numeric_limits<size_t>::max();
size_t TaskManager::computeExecutorIndex = std::numeric_limits<size_t>::max();
size_t TaskManager::transferExecutorIndex = std::numeric_limits<size_t>::max();

namespace {
auto setExecutorIndies(PurposeBits requirements,
                       size_t executorCount,
                       size_t& renderIdx,
                       size_t& transferIdx,
                       size_t& computeIdx) -> void
{
    auto lastAvailableDedicatedExecutorIdx = size_t{ 1 }; // main thread executor is counted

    auto dedicatedTransferRequired = requirements.test(Purpose::Transfer);
    auto dedicatedComputeRequired = requirements.test(Purpose::Compute);
    auto dedicatedRenderRequired = requirements.test(Purpose::Render);

    if (dedicatedRenderRequired && lastAvailableDedicatedExecutorIdx < executorCount) {
        renderIdx = lastAvailableDedicatedExecutorIdx++;
    }

    if (dedicatedTransferRequired && lastAvailableDedicatedExecutorIdx < executorCount) {
        transferIdx = lastAvailableDedicatedExecutorIdx++;
    } else {
        transferIdx = renderIdx;
    }

    if (dedicatedComputeRequired && lastAvailableDedicatedExecutorIdx < executorCount) {
        computeIdx = lastAvailableDedicatedExecutorIdx++;
    } else {
        computeIdx = renderIdx;
    }
}

auto executorPurpose(size_t i, size_t renderIdx, size_t transferIdx, size_t computeIdx) -> PurposeBits
{
    auto purposeBits = PurposeBits{ Purpose::General };

    if (i == renderIdx)
        purposeBits.set(Purpose::Render);

    if (i == transferIdx)
        purposeBits.set(Purpose::Transfer);

    if (i == computeIdx)
        purposeBits.set(Purpose::Compute);

    return purposeBits;
}
}

TaskManager::TaskManager(bool dedicatedTransferRequired,
                         bool dedicatedComputeRequired,
                         size_t threadPoolSize /*= std::max(std::thread::hardware_concurrency(), 1u)*/)
  : threadPool_{}
  , executorCount_{ threadPoolSize + 1 } // plus main thread
  , executors_{ std::make_unique_for_overwrite<Executor[]>(executorCount_) }
  , executorPurposes_{ std::make_unique_for_overwrite<PurposeBits[]>(executorCount_) }
  , executorIndexToStealTask_{ 0 }
  , taskPoolForStrand_{ config_t::strand_queue_max_size_v }
  , strandDeque_{ nullptr }
  , alive_{ true }
  , noTasks_{ true }
  , noTaskCv_{}
  , noTaskLock_{}
#if !defined(DISABLE_THREAD_EXCEPTIONS_PROPAGATION)
  , exPropagationLock_{}
  , exceptions_{}
#endif
{
    threadPool_.reserve(threadPoolSize);

    auto executorRequirements = PurposeBits{ Purpose::Render };
    if (dedicatedComputeRequired)
        executorRequirements.set(Purpose::Compute);
    if (dedicatedTransferRequired)
        executorRequirements.set(Purpose::Transfer);

    assert(executorCount_ >= 2);

    setExecutorIndies(
      executorRequirements, executorCount_, renderExecutorIndex, transferExecutorIndex, computeExecutorIndex);

    for (auto i = size_t{ 0 }; i < executorCount_; i++) {
        executorPurposes_[i] = executorPurpose(i, renderExecutorIndex, transferExecutorIndex, computeExecutorIndex);
        new (&executors_[i]) Executor{ *this };
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
            threadPool_.emplace_back([](Executor& executor, PurposeBits purpose) -> void { executor(purpose); },
                                     std::ref(executors_[i]),
                                     executorPurposes_[i]);
        }
    }
}

void TaskManager::stop()
{
    alive_.store(false, std::memory_order_release);

    // submit empty task to avoid executor deadlock
    Executor::threadExecutor().submitTask([]() -> void {});

    for (auto&& thread : threadPool_) {
        if (thread.joinable())
            thread.join();
    }
}

auto TaskManager::executorIndexToStealTask() -> size_t
{
    return executorIndexToStealTask_.fetch_add(1, std::memory_order_acq_rel) % executorCount_;
}

void TaskManager::waitForTasks()
{
    auto lk = std::unique_lock{ noTaskLock_ };
    noTaskCv_.wait(lk, [&]() -> bool { return !noTasks_; });
}

void TaskManager::notifyNewTask()
{
    auto lg = std::lock_guard{ noTaskLock_ };
    noTasks_ = false;
    noTaskCv_.notify_all();
}

void TaskManager::notifyNoTasks()
{
    auto lg = std::lock_guard{ noTaskLock_ };
    noTasks_ = true;
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
