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
    explicit TaskManager(size_t countThreads = std::thread::hardware_concurrency());

    TaskManager(TaskManager const&) = delete;

    TaskManager(TaskManager&&) = delete;

    ~TaskManager();

    auto operator=(TaskManager const&) -> TaskManager& = delete;

    auto operator=(TaskManager &&) -> TaskManager& = delete;

    template<typename Task>
    auto submit(Task&& task) const -> std::future<std::result_of_t<Task()>>;

    template<typename Task>
    auto strand(Task&& task) const -> std::future<std::result_of_t<Task()>>;

private:
    boost::asio::io_context ioContext_;

    boost::asio::io_context::strand strand_;

    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_;

    std::vector<std::thread> pool_;
};

template<typename Task>
auto TaskManager::submit(Task&& task) const -> std::future<std::result_of_t<Task()>>
{
    using result_type = std::result_of_t<Task()>;

    std::packaged_task<result_type()> packagedTask(std::forward<Task>(task));

    auto result = packagedTask.get_future();

    boost::asio::post(ioContext_, std::move(packagedTask));

    return result;
}

template<typename Task>
auto TaskManager::strand(Task&& task) const -> std::future<std::result_of_t<Task()>>
{
    using result_type = std::result_of_t<Task()>;

    std::packaged_task<result_type()> packagedTask(std::forward<Task>(task));

    auto result = packagedTask.get_future();

    boost::asio::post(strand_, std::move(packagedTask));

    return result;
}
}

#endif // CYCLONITE_TASKMANAGER_H
