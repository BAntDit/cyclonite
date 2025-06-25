
#include <utility>

template<size_t Size>
auto RingRange<Size>::expectedOffset(size_t size, bool forceShiftToBegin /* = false*/) const
  -> std::pair<size_t, size_t>
{
    auto expected = std::numeric_limits<size_t>::max();
    auto available = size_t{ 0 };

    auto nooverflow = [](size_t& wo, size_t& ro) -> void {
        auto reserved = wo - ro;
        ro = ro % Size;
        wo = ro + reserved;
    };

    auto avail = [](size_t wo, size_t ro) -> size_t {
        auto tempFree = Size - (wo - ro);
        auto two = wo % Size;
        auto tro = ro % Size;

        return tempFree == 0 ? 0 : two >= tro ? Size - two : tro - two;
    };

    if (size > 0) [[likely]] {
        if (contiguousFreeSize() >= size && !forceShiftToBegin) {
            auto tempWriteOffset = writeOffset_;
            auto tempReadOffset = readOffset_;
            if ((std::numeric_limits<size_t>::max() - tempWriteOffset) < size) {
                nooverflow(tempWriteOffset, tempReadOffset);
            }

            available = avail(tempWriteOffset, tempReadOffset);
            expected = tempWriteOffset;
        } else if (freeSize() > size && readOffset() >= size) {
            auto tempWriteOffset = writeOffset_;
            auto tempReadOffset = readOffset_;
            if (auto d = Size - (tempWriteOffset % Size); (std::numeric_limits<size_t>::max() - tempWriteOffset) < d) {
                nooverflow(tempWriteOffset, tempReadOffset);
            }

            tempWriteOffset += d;
            assert((tempWriteOffset % Size) == 0);

            if ((std::numeric_limits<size_t>::max() - tempWriteOffset) < size) {
                nooverflow(tempWriteOffset, tempReadOffset);
            }

            available = avail(tempWriteOffset, tempReadOffset);
            expected = tempWriteOffset;
        }
    }

    return std::pair{ expected, available };
}

template<size_t Size>
auto RingRange<Size>::contiguousFreeSize() const -> size_t
{
    return freeSize() == 0 ? 0 : (writeOffset() >= readOffset() ? Size - writeOffset() : readOffset() - writeOffset());
}

template<size_t Size>
auto RingRange<Size>::reserveRange(size_t size, bool forceShiftToBegin /* = false*/) -> size_t
{
    auto offset = std::numeric_limits<size_t>::max();

    if (size > 0) [[likely]] {
        if (contiguousFreeSize() >= size && !forceShiftToBegin) {
            if ((std::numeric_limits<size_t>::max() - writeOffset_) < size) {
                avoidOverflow();
            }
            offset = writeOffset();
            writeOffset_ += size;
            reservations_.emplace_back(size);
        } else if (freeSize() >= size && readOffset() >= size) {
            auto d = Size - writeOffset();
            if ((std::numeric_limits<size_t>::max() - writeOffset_) < d) {
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
    }

    return offset;
}

template<size_t Size>
auto RingRange<Size>::popRange() -> std::pair<size_t, size_t>
{
    assert(!reservations_.empty());

    auto size = reservations_.front();
    reservations_.pop_front();

    assert(size <= readableSize());

    auto offset = readOffset();
    readOffset_ += size;

    return std::pair{ offset, size };
}

template<size_t Size>
void RingRange<Size>::avoidOverflow()
{
    auto reserved = writeOffset_ - readOffset_;
    readOffset_ = readOffset();
    writeOffset_ = readOffset_ + reserved;
}
