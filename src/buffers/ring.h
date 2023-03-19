//
// Created by anton on 3/13/23.
//

#ifndef CYCLONITE_RING_H
#define CYCLONITE_RING_H

#include "allocatedMemory.h"
#include <cstddef>
#include <limits>

namespace cyclonite::buffers {
template<typename MemoryPage>
class Ring
{
public:
    Ring(size_t size, size_t align) noexcept;

    Ring(Ring const&) = delete;

    Ring(Ring&&) = default;

    ~Ring() = default;

    auto operator=(Ring const&) -> Ring& = delete;

    auto operator=(Ring&&) -> Ring& = default;

    [[nodiscard]] auto size() const -> size_t { return size_; }

    [[nodiscard]] auto ptr() const -> void const*;

    [[nodiscard]] auto ptr() -> void*;

    [[nodiscard]] auto currentMaxAvailableRange() const -> size_t;

    [[nodiscard]] auto alloc(size_t size) -> AllocatedMemory<MemoryPage>;

    void free(AllocatedMemory<MemoryPage> const& allocatedMemory);

    bool free(size_t offset, size_t size);

private:
    size_t size_;
    size_t align_;
    size_t head_;
    size_t tail_;
};

template<typename MemoryPage>
Ring<MemoryPage>::Ring(size_t size, size_t align) noexcept
  : size_{ size }
  , align_{ align }
  , head_{ 0 }
  , tail_{ 0 }
{
}

template<typename MemoryPage>
auto Ring<MemoryPage>::ptr() const -> void const*
{
    return static_cast<MemoryPage const*>(this)->ptr();
}

template<typename MemoryPage>
auto Ring<MemoryPage>::ptr() -> void*
{
    return static_cast<MemoryPage const*>(this)->ptr();
}

template<typename MemoryPage>
auto Ring<MemoryPage>::currentMaxAvailableRange() const -> size_t
{
    return (head_ > tail_ || (tail_ == 0 && head_ == 0)) ? std::max(size_ - head_, tail_) : tail_ - head_;
}

template<typename MemoryPage>
auto Ring<MemoryPage>::alloc(size_t size) -> AllocatedMemory<MemoryPage>
{
    auto actualSize = (size % align_) ? size + align_ - (size % align_) : size;
    auto offset = std::numeric_limits<size_t>::max();

    if (head_ > tail_ || (head_ == 0 && tail_ == 0) /* empty */) {
        assert(head_ <= size_);

        if ((size_ - head_) >= actualSize) {
            offset = head_;
            head_ = head_ + actualSize;
        } else if (tail_ >= actualSize) {
            offset = 0;
            head_ = actualSize;
        }
    } else if (head_ < tail_) {
        if ((tail_ - head_) >= actualSize) {
            offset = head_;
            head_ = head_ + actualSize;
        }
    }

    if (offset == std::numeric_limits<size_t>::max())
        actualSize = 0; // allocation is failed

    return AllocatedMemory<MemoryPage>{ *(static_cast<MemoryPage*>(this)), offset, actualSize };
}

template<typename MemoryPage>
void Ring<MemoryPage>::free(AllocatedMemory<MemoryPage> const& allocatedMemory)
{
    (void)allocatedMemory;
}

template<typename MemoryPage>
bool Ring<MemoryPage>::free(size_t offset, size_t size)
{
    auto success = false;
    if (tail_ == offset) {
        if ((tail_ += size) == size_)
            tail_ = 0;

        success = true;
    }
    return success;
}
}

#endif // CYCLONITE_RING_H
