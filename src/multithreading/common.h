//
// Created by anton on 9/8/25.
//

#ifndef CYCLONITE_MULTITHREADING_COMMON_H
#define CYCLONITE_MULTITHREADING_COMMON_H

#include <cassert>
#include <cstddef>
#include <future>
#include <memory>
#include <new>
#include <thread>
#include <type_traits>
#include <algorithm>

#if !defined(DISABLE_THREAD_EXCEPTIONS_PROPAGATION)
#include <exception>
#include <mutex>
#endif // ALLOW_THREAD_EXCEPTIONS_PROPAGATION

#ifdef __cpp_lib_hardware_interference_size
using std::hardware_constructive_interference_size;
using std::hardware_destructive_interference_size;
#else
// https://en.cppreference.com/w/cpp/thread/hardware_destructive_interference_size
// 64 bytes on x86-64 | L1_CACHE_BYTES | L1_CACHE_SHIFT | __cacheline_aligned | ...
inline constexpr size_t hardware_constructive_interference_size = 64;
inline constexpr size_t hardware_destructive_interference_size = 64;
#endif

namespace cyclonite::multithreading {
template<typename T>
concept DequeItemConcept = (std::is_nothrow_move_assignable_v<T> && std::is_nothrow_move_constructible_v<T> &&
                            std::is_nothrow_copy_assignable_v<T> && std::is_nothrow_copy_constructible_v<T>);

namespace internal {
template<DequeItemConcept DequeItemType>
class DequeData
{
public:
    explicit DequeData(size_t capacity);

    [[nodiscard]] auto capacity() const -> size_t { return capacity_; }

    void store(size_t index, DequeItemType&& t) noexcept;

    auto load(size_t index) const noexcept -> DequeItemType;

private:
    size_t capacity_;
    std::unique_ptr<DequeItemType[]> data_;
};

template<DequeItemConcept DequeItemType>
DequeData<DequeItemType>::DequeData(size_t capacity)
  : capacity_{ capacity }
  , data_{ std::make_unique_for_overwrite<DequeItemType[]>(capacity) }
{
}

template<DequeItemConcept DequeItemType>
void DequeData<DequeItemType>::store(size_t index, DequeItemType&& t) noexcept
{
    data_[index % capacity_] = std::move(t);
}

template<DequeItemConcept DequeItemType>
auto DequeData<DequeItemType>::load(size_t index) const noexcept -> DequeItemType
{
    return data_[index % capacity_];
}
} // internal

enum class TaskPurpose : uint_fast8_t
{
    General = 0,
    Render = 1
};
}
#endif // CYCLONITE_MULTITHREADING__COMMON_H
