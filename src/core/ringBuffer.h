//
// Created by anton on 6/19/25.
//

#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <cstddef>
#include <list>
#include <limits>
#include <cassert>

namespace cyclonite::core {
template<size_t Size>
class RingRange
{
public:
    RingRange() = default;

    [[nodiscard]] auto freeSize() const -> size_t { return Size - (writeOffset_ - readOffset_); }

    [[nodiscard]] auto contiguousFreeSize() const -> size_t;

    [[nodiscard]] auto readableSize() const -> size_t { return writeOffset_ - readOffset_; }

    // TODO:: force shift and expected offset
    auto reserveRange(size_t size) -> size_t;

    void popRange();

private:
    void avoidOverflow();

    [[nodiscard]] auto writeOffset() const -> size_t { return writeOffset_ % Size; }

    [[nodiscard]] auto readOffset() const -> size_t { return writeOffset_ % Size; }

    size_t writeOffset_;
    size_t readOffset_;
    std::list<size_t> reservations_;
};
// TODO:: to inl
template<size_t Size>
auto RingRange<Size>::contiguousFreeSize() const -> size_t
{
    return freeSize() == 0 ? 0 : (writeOffset() >= readOffset() ? Size - writeOffset() : readOffset() - writeOffset());
}

template<size_t Size>
auto RingRange<Size>::reserveRange(size_t size) -> size_t
{
    auto offset = std::numeric_limits<size_t>::max();

    if (contiguousFreeSize() >= size) {
        if ((std::numeric_limits<size_t>::max() - writeOffset_) < size) {
            avoidOverflow();
        }
        offset = writeOffset();
        writeOffset_ += size;
        reservations_.emplace_back(size);
    } else if (freeSize() >= size && readOffset() >= size) {
        assert(readOffset() >= writeOffset());
        if (auto d = Size - writeOffset(); (std::numeric_limits<size_t>::max() - writeOffset_) < d) {
            avoidOverflow();
        }
        writeOffset_ += d;
        assert(writeOffset() == 0);

        if ((std::numeric_limits<size_t>::max() - writeOffset_) < size) {
            avoidOverflow();
        }

        offset = writeOffset();
        writeOffset_ += size;
        reservations_.emplace_back(size + d);
    }

    return offset;
}

template<size_t Size>
void RingRange<Size>::popRange() {
    assert(!reservations_.empty());

    auto size = reservations_.back();
    reservations_.pop_back();

    assert(size <= readableSize());
    readOffset_ += size;
}

template<size_t Size>
void RingRange<Size>::avoidOverflow() {
    auto reserved = writeOffset_ - readOffset_;
    readOffset_ = readOffset();
    writeOffset_ = readOffset_ + reserved;
}
}

#endif // RINGBUFFER_H
