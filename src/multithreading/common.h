//
// Created by anton on 9/8/25.
//

#ifndef CYCLONITE_MULTITHREADING_COMMON_H
#define CYCLONITE_MULTITHREADING_COMMON_H

#include <cassert>
#include <cstddef>
#include <memory>
#include <new>
#include <type_traits>

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

inline constexpr size_t storage_align_v = alignof(void (*)());
inline constexpr size_t storage_size_v = 64;

struct functor_base_t
{
    functor_base_t() = default;

    virtual ~functor_base_t() = default;

    virtual void invoke() = 0;

    virtual auto move_to(std::byte (&storage)[storage_size_v]) -> functor_base_t* = 0;
};

template<typename F>
struct functor_t final : functor_base_t
{
    explicit functor_t(F const& f)
      : f_(f)
    {
    }

    explicit functor_t(F&& f)
      : f_(std::move(f))
    {
    }

    void invoke() override { f_(); }

    auto move_to(std::byte (&storage)[storage_size_v]) -> functor_base_t* override;

private:
    F f_;
};

template<typename F>
auto functor_t<F>::move_to(std::byte (&storage)[storage_size_v]) -> functor_base_t*
{
    auto* r = std::add_pointer_t<functor_base_t>{ nullptr };

    void* sdata = std::data(storage);
    size_t ssize = std::size(storage);

    if (auto* p = std::align(alignof(functor_t<F>), sizeof(functor_t<F>), sdata, ssize); p == std::data(storage)) {
        r = new (p) functor_t<F>{ std::move(f_) };
    }

    assert(r != nullptr);
    return r;
}
} // internal
}
#endif // CYCLONITE_MULTITHREADING__COMMON_H
