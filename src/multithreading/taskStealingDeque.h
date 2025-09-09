//
// Created by anton on 9/9/25.
//

#ifndef CYCLONITE_TASK_STEALING_DEQUE_H
#define CYCLONITE_TASK_STEALING_DEQUE_H

#include "common.h"
#include <optional>
#include <atomic>
#include <cstdint>
#include <limits>

namespace cyclonite::multithreading {
// circular work-stealing deque, SPMC and lock free
// the same idea as in paper below, but with static size ring
// https://www.dre.vanderbilt.edu/~schmidt/PDF/work-stealing-dequeue.pdf
template<DequeItemConcept DataItemType>
class TaskStealingDeque
{
public:
    explicit TaskStealingDeque(size_t capacity);

    ~TaskStealingDeque();

    [[nodiscard]] auto capacity() const -> size_t { return data_->capacity(); }

    [[nodiscard]] auto isEmpty() const -> bool;

    // can be called in producer thread only
    template<typename... Args>
    auto tryEmplace(Args&&... args) -> bool;

    // can be called in any thread, but
    // it's maden in purpose of consumer threads
    // in producer thread better use TryPop method instead
    auto trySteal() -> std::optional<DataItemType>;

    // can be called in producer thread only
    auto tryPop() -> std::optional<DataItemType>;

private:
    [[nodiscard]] auto countItems() const -> size_t;

    alignas(hardware_destructive_interference_size) std::atomic<uint64_t> top_;
    alignas(hardware_destructive_interference_size) std::atomic<uint64_t> bottom_;
    alignas(hardware_destructive_interference_size) internal::DequeData<DataItemType>* data_;
};

template<DequeItemConcept DataItemType>
TaskStealingDeque<DataItemType>::TaskStealingDeque(size_t capacity)
  : top_{ 0 }
  , bottom_{ 0 }
  , data_{ new internal::DequeData<DataItemType>{ capacity } }
{
}

template<DequeItemConcept DataItemType>
TaskStealingDeque<DataItemType>::~TaskStealingDeque()
{
    delete data_;
}

template<DequeItemConcept DataItemType>
auto TaskStealingDeque<DataItemType>::isEmpty() const -> bool
{
    std::atomic_thread_fence(std::memory_order_release);

    auto bottom = bottom_.load(std::memory_order_acquire);
    auto top = top_.load(std::memory_order_acquire);

    return top == bottom;
}

template<DequeItemConcept DataItemType>
auto TaskStealingDeque<DataItemType>::countItems() const -> size_t
{
    auto bottom = bottom_.load(std::memory_order_relaxed);
    auto top = top_.load(std::memory_order_acquire);

    return static_cast<size_t>((bottom >= top) ? bottom - top : (std::numeric_limits<uint64_t>::max() - top + bottom));
}

template<DequeItemConcept DataItemType>
template<typename... Args>
auto TaskStealingDeque<DataItemType>::tryEmplace(Args&&... args) -> bool
{
    if (countItems() < capacity()) {
        data_->store(static_cast<size_t>(bottom_.load(std::memory_order_relaxed)), T(std::forward<Args>(args)...));
        bottom_.fetch_add(1, std::memory_order_relaxed);
        return true;
    }

    return false;
}

template<DequeItemConcept DataItemType>
auto TaskStealingDeque<DataItemType>::trySteal() -> std::optional<DataItemType>
{
    auto result = std::optional<DataItemType>{ std::nullopt };

    auto top = top_.load(std::memory_order_acquire);

    std::atomic_thread_fence(std::memory_order_release);
    auto bottom = bottom_.load(std::memory_order_acquire);

    if (top != bottom) { // not empty
        result = data_->load(static_cast<size_t>(top));

        if (!top_.compare_exchange_strong(top, top + 1, std::memory_order_acq_rel, std::memory_order_relaxed)) {
            result = std::nullopt;
        }
    }

    return result;
}

template<DequeItemConcept DataItemType>
auto TaskStealingDeque<DataItemType>::tryPop() -> std::optional<DataItemType>
{
    auto result = std::optional<DataItemType>{ std::nullopt };

    auto bottom = bottom_.load(std::memory_order_relaxed);
    auto top = top_.load(std::memory_order_acquire);

    if (bottom != top) { // not empty
        auto prev = bottom--;
        bottom_.store(bottom, std::memory_order_release);

        if (prev == top_.load(std::memory_order_acquire)) { // someone stole everything
            bottom_.store(bottom + 1, std::memory_order_relaxed);
        } else {
            result = data_->load(static_cast<size_t>(bottom));
        }
    }

    return result;
}
}

#endif // CYCLONITE_TASK_STEALING_DEQUE_H
