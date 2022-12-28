//
// Created by bantdit on 1/19/19.
//

#include "taskManager.h"
#include "render.h"

namespace cyclonite::multithreading {
static constexpr auto _taskPoolSize = size_t{ 1024 };

TaskManager::TaskManager(size_t workerCount)
  : exceptions_{}
  , threadPool_{}
  , workers_{ std::make_unique_for_overwrite<Worker[]>(workerCount) }
  , workerCount_{ workerCount }
  , render_{ *this, _taskPoolSize }
  , exceptionMutex_{}
  , alive_{ true }
{
    for (auto i = size_t{ 0 }; i < workerCount_; i++) {
        new (&workers_[i]) Worker{ *this, _taskPoolSize };
    }

    threadPool_.reserve(workerCount_);
    threadPool_.emplace_back([](Render& render) -> void { render(); }, std::ref(render_));

    exceptions_.reserve(workerCount);
}

TaskManager::~TaskManager()
{
    stop();
}

void TaskManager::stop()
{
    alive_.store(false);

    for (auto&& thread : threadPool_) {
        if (thread.joinable())
            thread.join();
    }
}

void TaskManager::propagateException(std::exception_ptr const& exception)
{
    std::lock_guard<std::mutex> lock{ exceptionMutex_ };
    exceptions_.push_back(exception);
}

auto TaskManager::getLastException() -> std::exception_ptr
{
    std::lock_guard<std::mutex> lock{ exceptionMutex_ };

    auto ex = std::exception_ptr{};

    if (!exceptions_.empty()) {
        ex = exceptions_.back();
        exceptions_.pop_back();
    }

    return ex;
}
}
