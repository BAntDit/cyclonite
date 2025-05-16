//
// Created by bantdit on 10/26/22.
//

#ifndef CYCLONITE_LOCKFREEQUEUE_H
#define CYCLONITE_LOCKFREEQUEUE_H

#include "typedefs.h"
#include <atomic>
#include <bit>
#include <boost/lockfree/queue.hpp>
#include <cassert>
#include <cstddef>
#include <memory>
#include <new>
#include <optional>
#include <vector>

namespace cyclonite::multithreading {
// Dynamic Circular Work-Stealing Deque
// https://www.dre.vanderbilt.edu/~schmidt/PDF/work-stealing-dequeue.pdf
template<typename T>
class LockFreeSPMCQueue
{
    class CircularArray
    {
    public:
        explicit CircularArray(size_t capacity);

        [[nodiscard]] auto capacity() const -> size_t { return static_cast<size_t>(capacity_); }

        void store(size_t index, T&& t) noexcept
            requires std::is_nothrow_move_assignable_v<T>;

        auto load(size_t index) noexcept -> T
            requires std::is_nothrow_move_constructible_v<T>;

        [[nodiscard]] auto resize(int64_t bottom, int64_t top) -> CircularArray*;

    private:
        int64_t capacity_;
        int64_t mask_;
        std::unique_ptr<T[]> data_;
    };

public:
    explicit LockFreeSPMCQueue(size_t capacity);

    [[nodiscard]] auto size() const -> size_t;

    [[nodiscard]] auto capacity() const -> size_t;

    [[nodiscard]] auto empty() const -> bool { return size() == 0; }

    template<typename... Args>
    void emplaceBottom(Args&&... args);

    auto popBottom() -> std::optional<T>;

    auto steal() -> std::optional<T>;

    ~LockFreeSPMCQueue() noexcept;

private:
    alignas(hardware_destructive_interference_size) std::atomic<int64_t> top_;
    alignas(hardware_destructive_interference_size) std::atomic<int64_t> bottom_;
    alignas(hardware_destructive_interference_size) std::atomic<CircularArray*> buffer_;

    std::vector<std::unique_ptr<CircularArray>> garbage_;
};

template<typename T>
LockFreeSPMCQueue<T>::CircularArray::CircularArray(size_t capacity)
  : capacity_{ static_cast<int64_t>(capacity) }
  , mask_{ static_cast<int64_t>(capacity) - 1 }
  , data_{ std::make_unique_for_overwrite<T[]>(capacity) }
{
    assert(capacity > 0);
    assert(std::has_single_bit(capacity));
}

template<typename T>
void LockFreeSPMCQueue<T>::CircularArray::store(size_t index, T&& t) noexcept
    requires std::is_nothrow_move_assignable_v<T>
{
    data_[static_cast<int64_t>(index) & mask_] = std::move(t);
}

template<typename T>
auto LockFreeSPMCQueue<T>::CircularArray::load(size_t index) noexcept -> T
    requires std::is_nothrow_move_constructible_v<T>
{
    return data_[static_cast<int64_t>(index) & mask_];
}

template<typename T>
auto LockFreeSPMCQueue<T>::CircularArray::resize(int64_t bottom, int64_t top) -> CircularArray*
{
    auto* ptr = new CircularArray{ static_cast<size_t>(capacity_ * 2u) };

    for (auto i = top; i != bottom; i++) {
        ptr->store(static_cast<size_t>(i), load(static_cast<size_t>(i)));
    }

    return ptr;
}

template<typename T>
LockFreeSPMCQueue<T>::LockFreeSPMCQueue(size_t capacity)
  : top_{ 0 }
  , bottom_{ 0 }
  , buffer_{ new CircularArray{ capacity } }
  , garbage_{}
{
    garbage_.reserve(32);
}

template<typename T>
auto LockFreeSPMCQueue<T>::size() const -> size_t
{
    auto bottom = bottom_.load(std::memory_order_relaxed);
    auto top = top_.load(std::memory_order_relaxed);

    return static_cast<size_t>(bottom > top ? bottom - top : 0);
}

template<typename T>
auto LockFreeSPMCQueue<T>::capacity() const -> size_t
{
    return buffer_.load(std::memory_order_relaxed)->capacity();
}

template<typename T>
template<typename... Args>
void LockFreeSPMCQueue<T>::emplaceBottom(Args&&... args)
{
    auto&& item = T(std::forward<Args>(args)...);

    auto bottom = bottom_.load(std::memory_order_relaxed);
    auto top = top_.load(std::memory_order_acquire);

    auto* buffer = buffer_.load(std::memory_order_relaxed);

    if (buffer->capacity() < static_cast<size_t>((bottom - top) + 1)) {
        garbage_.emplace_back(std::exchange(buffer, buffer->resize(bottom, top)));
        buffer_.store(buffer, std::memory_order_relaxed);
    }

    buffer->store(static_cast<size_t>(bottom), std::move(item));

    std::atomic_thread_fence(std::memory_order_release);

    bottom_.store(bottom + 1, std::memory_order_relaxed);
}

template<typename T>
auto LockFreeSPMCQueue<T>::popBottom() -> std::optional<T>
{
    auto bottom = bottom_.load(std::memory_order_relaxed) - 1;
    auto* buffer = buffer_.load(std::memory_order_relaxed);

    bottom_.store(bottom, std::memory_order_relaxed);

    std::atomic_thread_fence(std::memory_order_seq_cst);

    auto top = top_.load(std::memory_order_relaxed);

    auto result = std::optional<T>{ std::nullopt };

    if (top <= bottom) {
        result = buffer->load(static_cast<size_t>(bottom));

        if (top == bottom) {
            // last item myabe stolen before our write above (stored bottom - 1)
            if (!top_.compare_exchange_strong(top, top + 1, std::memory_order_seq_cst, std::memory_order_relaxed)) {
                result = std::nullopt;
            }
            bottom_.store(bottom + 1, std::memory_order_relaxed);
        }
    } else {
        bottom_.store(bottom + 1, std::memory_order_relaxed);
        result = std::nullopt;
    }

    return result;
}

template<typename T>
auto LockFreeSPMCQueue<T>::steal() -> std::optional<T>
{
    auto top = top_.load(std::memory_order_acquire);

    std::atomic_thread_fence(std::memory_order_seq_cst);

    auto bottom = bottom_.load(std::memory_order_acquire);

    auto result = std::optional<T>{ std::nullopt };

    if (top < bottom) {
        result = buffer_.load(std::memory_order_consume)->load(static_cast<size_t>(top));

        if (!top_.compare_exchange_strong(top, top + 1, std::memory_order_seq_cst, std::memory_order_relaxed)) {
            result = std::nullopt;
        }
    } else {
        result = std::nullopt;
    }

    return result;
}

template<typename T>
LockFreeSPMCQueue<T>::~LockFreeSPMCQueue() noexcept
{
    delete buffer_.load(std::memory_order_relaxed);
}

template<typename T>
using lock_free_spmc_queue_t = LockFreeSPMCQueue<T>;

template<typename T>
using lock_free_mpmc_queue_t = boost::lockfree::queue<T>;
}

#endif // CYCLONITE_LOCKFREEQUEUE_H
