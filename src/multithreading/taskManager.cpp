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
    auto itemsPerTask = countItems / pool_.size();

    assert(itemsPerTask > 0);

    auto tail = countItems % pool_.size();
    auto count = tail > 0 ? countItems / itemsPerTask + 1 : countItems / itemsPerTask;

    return { itemsPerTask, count };
}
}
