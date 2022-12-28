//
// Created by bantdit on 11/2/22.
//

#include "render.h"
#include "taskManager.h"

namespace cyclonite::multithreading {
static thread_local Render* _renderThread = nullptr;
auto Render::renderThread() -> Render&
{
    assert(_renderThread);
    return *_renderThread;
}

auto Render::isInRenderThread() -> bool
{
    return _renderThread;
}

Render::Render(cyclonite::multithreading::TaskManager& taskManager, size_t taskPoolSize)
  : taskManager_{ &taskManager }
  , taskPool_{ taskPoolSize }
  , taskQueue_{ taskPoolSize }
{}

void Render::operator()()
{
    assert(_renderThread == nullptr);
    _renderThread = this;

    try {
        while (taskManager().keepAlive()) {
            if (auto task = pendingTask()) {
                task.value()();
            } else {
                std::this_thread::yield();
            }
        }
    }
    catch(...)
    {
        taskManager().propagateException(std::current_exception());
    }
}

auto Render::pendingTask() -> std::optional<Task>
{
    auto task = std::optional<Task>{ std::nullopt };

    Task* taskPtr = nullptr;
    if (queue().pop(taskPtr)) {
        task = std::move(*taskPtr);
    }

    return task;
}
}