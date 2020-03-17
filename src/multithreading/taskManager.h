//
// Created by bantdit on 1/19/19.
//

#ifndef CYCLONITE_TASKMANAGER_H
#define CYCLONITE_TASKMANAGER_H

#include <boost/asio.hpp>
#include <thread>
#include <type_traits>
#include <vector>

namespace cyclonite::multithreading {

class TaskManager
{
public:
    explicit TaskManager(size_t countThreads = std::max(std::thread::hardware_concurrency(), 2u) - 1u);

    TaskManager(TaskManager const&) = delete;

    TaskManager(TaskManager&&) = delete;

    ~TaskManager();

    auto operator=(TaskManager const&) -> TaskManager& = delete;

    auto operator=(TaskManager &&) -> TaskManager& = delete;

    template<typename Task>
    auto submit(Task&& task) -> std::future<std::result_of_t<Task()>>;

    template<typename Task>
    auto strand(Task&& task) const -> std::future<std::result_of_t<Task()>>;

    auto threadCount() const -> size_t { return pool_.size(); }

    auto getTaskCount(size_t countItems) const -> std::pair<size_t, size_t>;

    auto pool() const -> std::vector<std::thread> const& { return pool_; }

private:
    boost::asio::io_context ioContext_;

    boost::asio::io_context::strand strand_;

    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_;

    std::vector<std::thread> pool_;
};

template<typename Task>
auto TaskManager::submit(Task&& task) -> std::future<std::result_of_t<Task()>>
{
    using result_type = std::result_of_t<Task()>;

    std::packaged_task<result_type()> packagedTask(std::forward<Task>(task));

    return boost::asio::post(ioContext_, std::move(packagedTask));
}

template<typename Task>
auto TaskManager::strand(Task&& task) const -> std::future<std::result_of_t<Task()>>
{
    using result_type = std::result_of_t<Task()>;

    std::packaged_task<result_type()> packagedTask(std::forward<Task>(task));

    return boost::asio::post(strand_, std::move(packagedTask));
}
}

#endif // CYCLONITE_TASKMANAGER_H
