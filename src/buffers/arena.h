//
// Created by bantdit on 10/2/19.
//

#ifndef CYCLONITE_ARENA_H
#define CYCLONITE_ARENA_H

#include "allocatedMemory.h"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <deque>

namespace cyclonite::buffers {
template<typename MemoryPage>
class Arena
{
public:
    explicit Arena(size_t size);

    Arena(Arena const&) = delete;

    Arena(Arena&&) = default;

    ~Arena() = default;

    auto operator=(Arena const&) -> Arena& = delete;

    auto operator=(Arena&&) -> Arena& = default;

    [[nodiscard]] auto size() const -> size_t { return size_; }

    [[nodiscard]] auto ptr() const -> void const*;

    [[nodiscard]] auto ptr() -> void*;

    [[nodiscard]] auto maxAvailableRange() const -> size_t;

    [[nodiscard]] auto alloc(size_t size) -> AllocatedMemory<MemoryPage>;

    void free(AllocatedMemory<MemoryPage> const& allocatedMemory);

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
auto Arena<MemoryPage>::alloc(size_t size) -> AllocatedMemory<MemoryPage>
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

    return AllocatedMemory<MemoryPage>{ *(static_cast<MemoryPage*>(this)), rangeOffset, rangeSize };
}

template<typename MemoryPage>
void Arena<MemoryPage>::free(AllocatedMemory<MemoryPage> const& allocatedMemory)
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
auto Arena<MemoryPage>::view(size_t count, size_t offset /* = 0*/, size_t stride /* = sizeof(DataType)*/)
  -> buffers::BufferView<DataType>
{
    return buffers::BufferView<DataType>{ ptr(), offset, count, stride };
}
}

#endif // CYCLONITE_ARENA_H
