//
// Created by bantdit on 10/2/19.
//

#ifndef CYCLONITE_ARENA_H
#define CYCLONITE_ARENA_H

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <deque>
#include "bufferView.h"

namespace cyclonite::buffers {
template<typename MemoryPage>
class Arena
{
public:
    class AllocatedMemory
    {
    public:
        friend class Arena<MemoryPage>;

        AllocatedMemory();

        AllocatedMemory(MemoryPage& memoryPage, size_t offset, size_t size);

        AllocatedMemory(AllocatedMemory const&) = delete;

        AllocatedMemory(AllocatedMemory&& allocatedMemory) noexcept;

        ~AllocatedMemory();

        auto operator=(AllocatedMemory const&) -> AllocatedMemory& = delete;

        auto operator=(AllocatedMemory&& rhs) noexcept -> AllocatedMemory&;

        explicit operator std::byte *() { return reinterpret_cast<std::byte*>(ptr_); }

        [[nodiscard]] auto ptr() const -> void const* { return ptr_; }

        [[nodiscard]] auto ptr() -> void* { return ptr_; }

        [[nodiscard]] auto size() const -> size_t { return size_; }

        [[nodiscard]] auto offset() const -> size_t { return offset_; }

        [[nodiscard]] auto memoryPage() const -> MemoryPage const& { return *memoryPage_; }

        template<typename DataType>
        auto view(size_t count, size_t offset = 0, size_t stride = sizeof(DataType)) -> buffers::BufferView<DataType>;

    private:
        MemoryPage* memoryPage_;
        void* ptr_;
        size_t offset_;
        size_t size_;
    };

public:
    explicit Arena(size_t size);

    Arena(Arena const&) = delete;

    Arena(Arena&&) = default;

    ~Arena() = default;

    auto operator=(Arena const&) -> Arena& = delete;

    auto operator=(Arena &&) -> Arena& = default;

    [[nodiscard]] auto size() const -> size_t { return size_; }

    [[nodiscard]] auto ptr() const -> void const*;

    [[nodiscard]] auto ptr() -> void*;

    [[nodiscard]] auto maxAvailableRange() const -> size_t;

    [[nodiscard]] auto alloc(size_t size) -> AllocatedMemory;

    void free(AllocatedMemory const& allocatedMemory);

    template<typename DataType>
    auto view(size_t count, size_t offset = 0, size_t stride = sizeof(DataType)) -> buffers::BufferView<DataType>;

protected:
    size_t size_;
    std::deque<std::pair<size_t, size_t>> freeRanges_;
};

template<typename MemoryPage>
Arena<MemoryPage>::Arena(size_t size)
  : size_{ size }
  , freeRanges_{}
{
    freeRanges_.emplace_back(size_t{ 0 }, size);
}

// calls in strand always
template<typename MemoryPage>
auto Arena<MemoryPage>::alloc(size_t size) -> Arena<MemoryPage>::AllocatedMemory
{

    auto it = std::lower_bound(freeRanges_.begin(), freeRanges_.end(), size, [](auto const& lhs, size_t rhs) -> bool {
        return lhs.second < rhs;
    });

    assert(it != freeRanges_.end());

    auto [rangeOffset, rangeSize] = (*it);

    freeRanges_.erase(it);

    if (rangeSize > size) {
        auto newOffset = rangeOffset + size;
        auto newSize = rangeSize - size;

        if (auto newIt = std::upper_bound(freeRanges_.begin(),
                                          freeRanges_.end(),
                                          newSize,
                                          [](size_t lhs, auto const& rhs) -> bool { return lhs < rhs.second; });
            newIt != freeRanges_.end()) {
            freeRanges_.emplace(newIt, newOffset, newSize);
        } else {
            freeRanges_.emplace_back(newOffset, newSize);
        }
    }

    return Arena<MemoryPage>::AllocatedMemory{ *(static_cast<MemoryPage*>(this)), rangeOffset, rangeSize };
}

template<typename MemoryPage>
void Arena<MemoryPage>::free(Arena<MemoryPage>::AllocatedMemory const& allocatedMemory)
{
    auto offset = allocatedMemory.offset();
    auto size = allocatedMemory.size();

    auto prevIt = std::find_if(
      freeRanges_.begin(), freeRanges_.end(), [=](auto const& p) -> bool { return p.first + p.second == offset; });

    auto nextIt = std::find_if(
      freeRanges_.begin(), freeRanges_.end(), [=](auto const& p) -> bool { return offset + size == p.first; });

    auto prevIndex = std::distance(freeRanges_.begin(), prevIt);
    auto nextIndex = std::distance(freeRanges_.begin(), nextIt);

    auto newOffset = offset;
    auto newSize = size;

    if (prevIt != freeRanges_.end() && nextIt != freeRanges_.end()) {
        assert(prevIndex != nextIndex);

        newOffset = (*prevIt).first;
        newSize = (*prevIt).second + size + (*nextIt).second;

        if (nextIndex > prevIndex) {
            freeRanges_.erase(std::next(freeRanges_.begin(), nextIndex));
            freeRanges_.erase(std::next(freeRanges_.begin(), prevIndex));
        } else {
            freeRanges_.erase(std::next(freeRanges_.begin(), prevIndex));
            freeRanges_.erase(std::next(freeRanges_.begin(), nextIndex));
        }
    } else if (prevIt != freeRanges_.end()) {
        newOffset = (*prevIt).first;
        newSize = (*prevIt).second + size;

        freeRanges_.erase(prevIt);
    } else if (nextIt != freeRanges_.end()) {
        newSize = size + (*nextIt).second;

        freeRanges_.erase(nextIt);
    }

    if (auto newIt = std::upper_bound(freeRanges_.begin(),
                                      freeRanges_.end(),
                                      newSize,
                                      [](size_t lhs, auto const& rhs) -> bool { return lhs < rhs.second; });
        newIt != freeRanges_.end()) {
        freeRanges_.emplace(newIt, newOffset, newSize);
    } else {
        freeRanges_.emplace_back(newOffset, newSize);
    }
}

template<typename MemoryPage>
auto Arena<MemoryPage>::ptr() const -> void const*
{
    return static_cast<MemoryPage const*>(this)->ptr();
}

template<typename MemoryPage>
auto Arena<MemoryPage>::ptr() -> void*
{
    return static_cast<MemoryPage const*>(this)->ptr();
}

template<typename MemoryPage>
auto Arena<MemoryPage>::maxAvailableRange() const -> size_t
{
    return freeRanges_.empty() ? 0 : static_cast<size_t>(freeRanges_.back().second);
}

template<typename MemoryPage>
template<typename DataType>
auto Arena<MemoryPage>::view(size_t count, size_t offset/* = 0*/, size_t stride/* = sizeof(DataType)*/) -> buffers::BufferView<DataType>
{
    return buffers::BufferView<DataType>{ ptr(), offset, count, stride };
}

template<typename MemoryPage>
Arena<MemoryPage>::AllocatedMemory::AllocatedMemory()
  : memoryPage_{ nullptr }
  , ptr_{ nullptr }
  , offset_{ 0 }
  , size_{ 0 }
{}

template<typename MemoryPage>
Arena<MemoryPage>::AllocatedMemory::AllocatedMemory(MemoryPage& memoryPage, size_t offset, size_t size)
  : memoryPage_{ &memoryPage }
  , ptr_{ memoryPage_->ptr() == nullptr ? nullptr : reinterpret_cast<std::byte*>(memoryPage_->ptr()) + offset }
  , offset_{ offset }
  , size_{ size }
{}

template<typename MemoryPage>
Arena<MemoryPage>::AllocatedMemory::AllocatedMemory(Arena<MemoryPage>::AllocatedMemory&& allocatedMemory) noexcept
  : memoryPage_{ allocatedMemory.memoryPage_ }
  , ptr_{ allocatedMemory.ptr_ }
  , offset_{ allocatedMemory.offset_ }
  , size_{ allocatedMemory.size_ }
{
    allocatedMemory.memoryPage_ = nullptr;
    allocatedMemory.ptr_ = nullptr;
    allocatedMemory.offset_ = 0;
    allocatedMemory.size_ = 0;
}

template<typename MemoryPage>
Arena<MemoryPage>::AllocatedMemory::~AllocatedMemory()
{
    if (memoryPage_ != nullptr) {
        memoryPage_->free(*this);
    }
}

template<typename MemoryPage>
auto Arena<MemoryPage>::AllocatedMemory::operator=(Arena<MemoryPage>::AllocatedMemory&& rhs) noexcept
  -> Arena<MemoryPage>::AllocatedMemory&
{
    memoryPage_ = rhs.memoryPage_;
    ptr_ = rhs.ptr_;
    offset_ = rhs.offset_;
    size_ = rhs.size_;

    rhs.memoryPage_ = nullptr;
    rhs.ptr_ = nullptr;
    rhs.offset_ = 0;
    rhs.size_ = 0;

    return *this;
}

template<typename MemoryPage>
template<typename DataType>
auto Arena<MemoryPage>::AllocatedMemory::view(size_t count,
                                              size_t offset /* = 0*/,
                                              size_t stride /* = sizeof(DataType)*/) -> buffers::BufferView<DataType>
{
    return buffers::BufferView<DataType>{ ptr(), offset, count, stride };
}
}

#endif // CYCLONITE_ARENA_H
