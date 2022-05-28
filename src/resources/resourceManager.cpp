//
// Created by anton on 5/28/22.
//

#include "resourceManager.h"

namespace cyclonite::resources {
ResourceManager::ResourceManager(size_t expectedResourceCount, size_t expectedDependencyCount)
  : resources_{}
  , freeResourceIndices_{}
  , dependencies_{}
  , storages_{}
  , freeRanges_{}
{
    resources_.reserve(expectedResourceCount);
    freeResourceIndices_.reserve(expectedResourceCount);
    dependencies_.reserve(expectedDependencyCount);
}

auto ResourceManager::allocResource(Resource::ResourceTag tag, size_t size) -> Resource::Id
{
    auto version = uint32_t{ 1 };
    auto index = std::numeric_limits<uint32_t>::max();

    if (!freeResourceIndices_.empty()) {
        index = freeResourceIndices_.back();
        freeResourceIndices_.pop_back();

        version = resources_[index].version;
    } else {
        index = resources_.size();
        resources_.emplace_back();
    }

    assert(index < resources_.size());

    auto& storage = storages_[tag.index];
    auto& ranges = freeRanges_[tag.index];

    auto it = std::lower_bound(ranges.cbegin(), ranges.cend(), size, [](auto range, auto value) -> bool {
        auto [rangeOffset, rangeSize] = range;
        (void)rangeOffset;

        return rangeSize < value;
    });

    if (it == ranges.cend()) {
        auto prevSize = storage.size();
        auto newRangeOffset = prevSize;
        auto newRangeSize = prevSize;

        auto mergeIt = std::find_if(ranges.cbegin(), ranges.cend(), [newRangeOffset](auto range) -> bool {
            return newRangeOffset == (range.first + range.second);
        });

        if (mergeIt != ranges.cend()) {
            auto [mergeOffset, mergeSize] = *mergeIt;
            newRangeOffset = mergeOffset;
            newRangeSize = newRangeSize + mergeSize;
            ranges.erase(mergeIt);
        }

        ranges.insert(std::pair{ static_cast<size_t>(newRangeOffset), static_cast<size_t>(newRangeSize) });

        storage.resize(prevSize * 2, std::byte{ 0 });

        it = std::lower_bound(ranges.cbegin(), ranges.cend(), size, [](auto range, auto value) -> bool {
            auto [rangeOffset, rangeSize] = range;
            (void)rangeOffset;

            return rangeSize < value;
        });
    }

    if (it == ranges.cend()) {
        throw std::runtime_error("no enough memory to place resource");
    }

    auto [allocOffset, allocSize] = *it;

    if (allocOffset > size) {
        auto newRangeOffset = allocOffset + allocSize;
        auto newRangeSize = allocSize - size;

        ranges.insert(std::pair{ static_cast<size_t>(newRangeOffset), static_cast<size_t>(newRangeSize) });
    }

    auto& resource = resources_[index];
    resource.version = version;
    resource.storage_index = tag.index;
    resource.is_fixed_size = tag.isFixedSizePerItem;
    resource.size = size;
    resource.offset = allocOffset;

    return Resource::Id{ index, version };
}

void ResourceManager::erase(Resource::Id id)
{
    auto& [offset, size, version, storageIndex, isFixed] = resources_[id.index()];

    version++;

    auto freeOffset = offset;
    auto freeSize = size;

    auto& storage = storages_[storageIndex];
    auto& ranges = freeRanges_[storageIndex];

    std::fill_n(storage.data() + offset, size, std::byte{ 0 });

    auto prevRange = std::find_if(ranges.cbegin(), ranges.cend(), [freeOffset](auto&& range) -> bool {
       auto&& [rangeOffset, rangeSize] = range;
       return freeOffset == (rangeOffset + rangeSize);
    });

    if (prevRange != ranges.cend()) {
        auto [prevOffset, prevSize] = *prevRange;
        freeOffset = prevOffset;
        freeSize = freeSize + prevSize;

        ranges.erase(prevRange);
    }

    auto nextRange = std::find_if(ranges.cbegin(), ranges.cend(), [freeOffset, freeSize](auto&& range) -> bool {
        auto&& [rangeOffset, rangeSize] = range;
        return rangeOffset == (freeOffset + freeSize);
    });

    if (nextRange != ranges.cend()) {
        auto [nextOffset, nextSize] = *nextRange;
        (void)nextOffset;
        freeSize = freeSize + nextSize;

        ranges.erase(prevRange);
    }

    ranges.insert(std::pair{ static_cast<size_t>(freeOffset), static_cast<size_t>(freeSize) });

    offset = std::numeric_limits<size_t>::max();
    size = 0;
    storageIndex = std::numeric_limits<uint16_t>::max();
    isFixed = false;
}
}