//
// Created by bantdit on 1/19/19.
//

#include "taskManager.h"

namespace cyclonite::multithreading {
TaskManager::TaskManager(size_t countThreads)
  : ioContext_{}
  , strand_{ ioContext_ }
  , work_{ boost::asio::make_work_guard(ioContext_) }
  , pool_{}
{
    while (countThreads-- > 0) {
        pool_.emplace_back([&]() -> void { ioContext_.run(); });
    }
}

TaskManager::~TaskManager()
{
    work_.reset();

    for (auto& thread : pool_) {
        thread.join();
    }

    pool_.clear();
}

auto TaskManager::getTaskCount(size_t countItems) const -> std::pair<size_t, size_t>
{
    assert(countItems > 0);

    auto itemsPerTask = size_t{ 0 };
    auto taskCount = size_t{ 0 };

    if (countItems < pool_.size()) {
        itemsPerTask = countItems;
        taskCount = 1;
    } else {
        auto tail = countItems % pool_.size();
        itemsPerTask = countItems / pool_.size();
        taskCount = (tail > 0) ? countItems / itemsPerTask + 1 : countItems / itemsPerTask;
    }

    return { taskCount, itemsPerTask };
}
}
