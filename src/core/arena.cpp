
#include "arena.h"
#include <cassert>
#include <memory>

namespace cyclonite::core {
Arena::Arena(size_t capacity)
  : freeRanges_{}
  , freeRangeOffsets_{}
  , capacity_{ capacity }
{
    auto it = freeRanges_.emplace(capacity_, 0);

    freeRangeOffsets_.emplace(0, it);
}

auto Arena::getAllocatedSize() const -> size_t
{
    auto freeSize = size_t{ 0 };
    for (auto [size, offset] : freeRanges_) {
        freeSize += size;
    }
    return capacity_ - freeSize;
}

auto Arena::AllocInternal(std::byte* const basePtr,
                          size_t requiredSize,
                          size_t offsetAlignment /* = 1*/,
                          size_t sizeAlignment /* = 1*/) -> AllocInfoInternal
{
    auto offset = std::numeric_limits<size_t>::max();
    auto align = std::numeric_limits<size_t>::max();
    auto size = size_t{ 0 };
    auto* outPtr = std::add_pointer_t<void>{ nullptr };
    bool makeSecondAttempt = false;

    {
        auto alignment = size_t{ 0 };
        if (sizeAlignment > 1) {
            if (auto r = requiredSize % sizeAlignment; r > 0) {
                alignment = sizeAlignment - r;
            }
        }
        requiredSize += alignment;
    }
    assert((requiredSize % sizeAlignment) == 0);

    auto it =
      std::lower_bound(freeRanges_.begin(), freeRanges_.end(), requiredSize, [](auto&& element, auto rs) -> bool {
          auto&& [sz, _] = element;
          return sz < rs;
      });

    if (it != freeRanges_.end()) {
        auto [rangeSize, rangeOffset] = *it;
        auto space = rangeSize;
        auto* ptr = reinterpret_cast<void*>(basePtr + rangeOffset);

        assert(space >= offsetAlignment);
        if (offsetAlignment > 1 && basePtr != nullptr) {
            outPtr = std::align(offsetAlignment, requiredSize, ptr, space);
            assert(rangeSize >= space);
        } else if (offsetAlignment > 1) {
            if (auto r = rangeOffset % sizeAlignment; r > 0) {
                space -= (sizeAlignment - r);
            }
            outPtr = ptr;
        } else {
            outPtr = ptr;
        }

        auto diff = rangeSize - space;
        align = diff;
        requiredSize += diff;

        if (outPtr != nullptr || requiredSize <= rangeSize) {
            offset = rangeOffset;
            size = requiredSize;

            assert(freeRangeOffsets_.count(rangeOffset) == 1);
            freeRangeOffsets_.erase(rangeOffset);
            freeRanges_.erase(it);

            if (rangeSize > requiredSize) {
                auto newSize = rangeSize - requiredSize;
                auto newOffset = rangeOffset + requiredSize;

                auto newIt = freeRanges_.emplace(newSize, newOffset);
                freeRangeOffsets_.emplace(newOffset, newIt);
            }
        } else {
            makeSecondAttempt = true;
        }
    }

    auto allocInfo = AllocInfoInternal{};
    allocInfo.ptr = outPtr;
    allocInfo.rangeOffset = offset;
    allocInfo.rangeSize = size;
    allocInfo.rangeAlign = align;
    allocInfo.makeNewAttempt = makeSecondAttempt;

    return allocInfo;
}

auto Arena::alloc(void* basePtr,
                  size_t requiredSize,
                  size_t offsetAlignment /*= 1*/,
                  size_t sizeAlignment /*= 1*/) -> Arena::AllocInfo
{
    auto offset = std::numeric_limits<size_t>::max();
    auto align = std::numeric_limits<size_t>::max();
    auto size = requiredSize;
    auto outPtr = std::add_pointer_t<void>{ nullptr };

    auto [allocPtr, rangeOffset, rangeSize, rangeAlign, makeSecondAttempt] =
      AllocInternal(reinterpret_cast<std::byte*>(basePtr), requiredSize, offsetAlignment, sizeAlignment);

    if (allocPtr != nullptr) {
        offset = rangeOffset;
        size = rangeSize;
        align = rangeAlign;
        outPtr = allocPtr;
    } else if (makeSecondAttempt) {
        requiredSize += rangeAlign;

        auto [allocPtr2, rangeOffset2, rangeSize2, rangeAlign2, _] =
          AllocInternal(reinterpret_cast<std::byte*>(basePtr), requiredSize, offsetAlignment, sizeAlignment);

        if (allocPtr2 != nullptr) {
            offset = rangeOffset2;
            size = rangeSize2;
            align = rangeAlign2;
            outPtr = allocPtr2;
        }
    }

    auto allocInfo = AllocInfo{};
    allocInfo.ptr = outPtr;
    allocInfo.rangeOffset = offset;
    allocInfo.rangeSize = size;
    allocInfo.rangeAlign = align;

    return allocInfo;
}

auto Arena::alloc(size_t requiredSize, size_t offsetAlignment /*= 1*/, size_t sizeAlignment /*= 1*/) -> Arena::AllocInfo
{
    auto offset = std::numeric_limits<size_t>::max();
    auto align = std::numeric_limits<size_t>::max();
    auto size = requiredSize;

    auto [_0, rangeOffset, rangeSize, rangeAlign, makeSecondAttempt] =
      AllocInternal(nullptr, requiredSize, offsetAlignment, sizeAlignment);

    if (rangeOffset != std::numeric_limits<size_t>::max()) {
        offset = rangeOffset;
        size = rangeSize;
        align = rangeAlign;
    } else if (makeSecondAttempt) {
        requiredSize += rangeAlign;

        auto [allocPtr2, rangeOffset2, rangeSize2, rangeAlign2, _] =
          AllocInternal(nullptr, requiredSize, offsetAlignment, sizeAlignment);

        if (allocPtr2 != nullptr) {
            offset = rangeOffset2;
            size = rangeSize2;
            align = rangeAlign2;
        }
    }

    auto allocInfo = AllocInfo{};
    allocInfo.ptr = nullptr;
    allocInfo.rangeOffset = offset;
    allocInfo.rangeSize = size;
    allocInfo.rangeAlign = align;

    return allocInfo;
}

void Arena::free(size_t offset, size_t size)
{
    assert((offset + size) <= capacity_);

    auto freeOffset = offset;
    auto freeSize = size;

    auto prevFragment = freeRangeOffsets_.end();
    auto nextFragment = freeRangeOffsets_.end();

    auto offsetIt = freeRangeOffsets_.lower_bound(offset);

    if (offsetIt != freeRangeOffsets_.begin()) {
        auto prevOffsetIt = std::prev(offsetIt);
        auto [prevOffset, prevFreeRangeIt] = *prevOffsetIt;
        auto [prevSize, _] = *prevFreeRangeIt;

        assert(prevOffset == _);
        if ((prevOffset + prevSize) == offset) {
            prevFragment = prevOffsetIt;
            freeOffset = prevOffset;
            freeSize += prevSize;
        }
    }

    auto nextOffsetIt = freeRangeOffsets_.lower_bound(offset + size);
    if (nextOffsetIt != freeRangeOffsets_.end()) {
        auto [nextOffset, nextFreeRangeIt] = *nextOffsetIt;

        if ((offset + size) == nextOffset) {
            auto [nextSize, _] = *nextFreeRangeIt;
            assert(nextOffset == _);

            nextFragment = nextOffsetIt;
            freeSize += nextSize;
        }
    }

    // partial defragmentation
    if (prevFragment != freeRangeOffsets_.end()) {
        auto [_, it] = *prevFragment;
        freeRangeOffsets_.erase(prevFragment);
        freeRanges_.erase(it);
    }

    // partial defragmentation
    if (nextFragment != freeRangeOffsets_.end()) {
        auto [_, it] = *nextFragment;
        freeRangeOffsets_.erase(nextFragment);
        freeRanges_.erase(it);
    }

    {
        auto it = freeRanges_.emplace(freeSize, freeOffset);
        freeRangeOffsets_.emplace(freeOffset, it);
    }
}

void Arena::freeAll()
{
    freeRangeOffsets_.clear();
    freeRanges_.clear();

    auto it = freeRanges_.emplace(capacity_, 0);
    freeRangeOffsets_.emplace(0, it);
}
}
