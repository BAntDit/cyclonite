
#include "taskManager.h"

namespace cyclonite::multithreading {
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
