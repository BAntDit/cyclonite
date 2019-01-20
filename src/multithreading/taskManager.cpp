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
}
