//
// Created by bantdit on 1/19/19.
//

#include "taskManager.h"

namespace cyclonite::multithreading {
static constexpr auto _taskPoolSize = size_t{ 1024 };

TaskManager::TaskManager(size_t workerCount)
  : threadPool_{}
  , workers_{ std::make_unique_for_overwrite<Worker[]>(workerCount) }
  , workerCount_{ workerCount }
  , render_{ *this, _taskPoolSize }
  , alive_{ false }
{
    for (auto i = size_t{ 0 }; i < workerCount_; i++) {
        new (&workers_[i]) Worker{ *this, _taskPoolSize };
    }
}

TaskManager::~TaskManager()
{
    alive_.store(false);

    for (auto&& thread : threadPool_) {
        thread.join();
    }
}
}
