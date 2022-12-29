//
// Created by bantdit on 10/30/22.
//

#ifndef CYCLONITE_RENDER_H
#define CYCLONITE_RENDER_H

#include "lockFreeQueue.h"
#include "taskPool.h"
#include <future>

namespace cyclonite::multithreading {
class TaskManager;

class Render
{
    friend class TaskManager;

public:
    explicit Render(TaskManager& taskManager) noexcept;

    Render(Render const&) = delete;

    Render(Render&&) = delete;

    ~Render() = default;

    auto operator=(Render const&) -> Render& = delete;

    auto operator=(Render &&) -> Render& = delete;

    void operator()();

    [[nodiscard]] auto taskManager() const -> TaskManager const& { return *taskManager_; }
    auto taskManager() -> TaskManager& { return *taskManager_; }

private:
    auto pendingTask() -> std::optional<Task>;

    static auto renderThread() -> Render&;

public:
    static auto isInRenderThread() -> bool;

private:
    TaskManager* taskManager_;
};
}

#endif // CYCLONITE_RENDER_H
