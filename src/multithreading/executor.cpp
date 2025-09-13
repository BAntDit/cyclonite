//
// Created by anton on 9/13/25.
//

#include "executor.h"

namespace cyclonite::multithreading
{
namespace {
thread_local Executor* _mainThreadExecutor = nullptr;
thread_local Executor* _threadExecutor = nullptr;
}

/*static*/ auto Executor::isInMainThread() -> bool
{
    return _mainThreadExecutor != nullptr;
}

/*static*/ auto Executor::threadExecutor() -> Executor&
{
    assert(_threadExecutor != nullptr);
    return *_threadExecutor;
}
}
