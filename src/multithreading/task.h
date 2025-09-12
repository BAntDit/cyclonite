//
// Created by anton on 9/9/25.
//

#ifndef CYCLONITE_MULTITHREADING_TASK_H
#define CYCLONITE_MULTITHREADING_TASK_H

#include "common.h"
#include <atomic>
#include <cassert>

namespace cyclonite::multithreading {
class Task;

template<typename T>
concept TaskFunctorConcept = !
std::is_same_v<std::decay_t<T>, Task>;

class alignas(hardware_destructive_interference_size) Task
{
public:
    Task();

    template<TaskFunctorConcept F>
    explicit Task(F&& f);

    Task(Task const&) = delete;

    Task(Task&& task) noexcept;

    auto operator=(Task const&) -> Task& = delete;

    auto operator=(Task&& rhs) noexcept -> Task&;

    void operator()();

    [[nodiscard]] auto pending() const -> bool { return pending_.load(std::memory_order_acquire); }

    ~Task();

private:
    void _reset();

    auto storage() -> void* { return storage_; }

private:
    alignas(
      internal::storage_align_v) std::byte storage_[internal::storage_size_v]; // for small functor optimization (sfo)
    internal::functor_base_t* functor_;
    std::atomic<bool> pending_;
};

template<TaskFunctorConcept F>
Task::Task(F&& f)
  : storage_{}
  , functor_{ nullptr }
  , pending_{ false }
{
    void* storage = std::data(storage_);
    auto size = std::size(storage_);

    if (auto* p = std::align(alignof(internal::functor_t<F>), sizeof(internal::functor_t<F>), storage, size);
        p == std::data(storage_)) {
        // in case functor is small enough and functor alignment matches with sfo buffer alignment
        functor_ = new (p) internal::functor_t<F>{ std::forward<F>(f) };
    } else {
        functor_ = new internal::functor_t<F>{ std::forward<F>(f) };
    }
    pending_.store(functor_ != nullptr, std::memory_order_relaxed);
}

class StrandTask;
}

#endif // CYCLONITE_MULTITHREADING__TASK_H
