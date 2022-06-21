//
// Created by anton on 5/28/22.
//

#include "resourceManager.h"

namespace cyclonite::resources {
ResourceManager::ResourceManager(size_t expectedResourceCount)
  : resources_{}
  , freeResourceIndices_{}
  , storages_{}
  , freeItems_{}
  , buffers_{}
  , freeRanges_{}
{
    resources_.reserve(expectedResourceCount);
    freeResourceIndices_.reserve(expectedResourceCount);
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

    auto& storage = storages_[tag.staticDataIndex];
    auto& items = freeItems_[tag.staticDataIndex];

    auto itemIndex = uint32_t{ 0 };

    if (items.size() > 1) {
        itemIndex = items.back();
        items.pop_back();
    } else {
        itemIndex = items.back()++;
    }

    if ((itemIndex * size) >= storage.size()) {
        storage.resize(storage.size() * 2, std::byte{ 0 });
    }

    auto& resource = resources_[index];
    resource.size = size;
    resource.version = version;
    resource.static_index = tag.staticDataIndex;
    resource.dynamic_index = tag.dynamicDataIndex;
    resource.item = itemIndex;

    return Resource::Id{ index, version };
}

void ResourceManager::resizeDynamicBuffer(Resource::ResourceTag tag, size_t additionalSize)
{
    assert(tag.dynamicDataIndex < buffers_.size());
    auto bufferIndex = tag.dynamicDataIndex;

    auto& buffer = buffers_[bufferIndex];
    auto& ranges = freeRanges_[bufferIndex];

    auto prevSize = buffer.size();
    auto newRangeOffset = prevSize;
    auto addSize = std::max(prevSize, additionalSize);
    auto newRangeSize = addSize;

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

    buffer.resize(prevSize + addSize, std::byte{ 0 });
}

auto ResourceManager::allocDynamicBuffer(Resource::ResourceTag tag, size_t size, bool resizeAllowed) -> size_t
{
    assert(tag.dynamicDataIndex < buffers_.size());
    auto bufferIndex = tag.dynamicDataIndex;

    auto& ranges = freeRanges_[bufferIndex];

    auto it = std::lower_bound(ranges.cbegin(), ranges.cend(), size, [](auto range, auto value) -> bool {
        auto [rangeOffset, rangeSize] = range;
        (void)rangeOffset;

        return rangeSize < value;
    });

    if (it == ranges.cend() && resizeAllowed) {
        resizeDynamicBuffer(tag, size);

        it = std::lower_bound(ranges.cbegin(), ranges.cend(), size, [](auto range, auto value) -> bool {
            auto [rangeOffset, rangeSize] = range;
            (void)rangeOffset;

            return rangeSize < value;
        });
    }

    if (it == ranges.cend()) {
        throw std::bad_alloc{};
    }

    auto [allocOffset, allocSize] = *it;

    ranges.erase(it);

    if (allocSize > size) {
        auto newRangeOffset = allocOffset + size;
        auto newRangeSize = allocSize - size;

        ranges.insert(std::pair{ static_cast<size_t>(newRangeOffset), static_cast<size_t>(newRangeSize) });
    }

    return allocOffset;
}

void ResourceManager::freeDynamicBuffer(uint16_t dynamicIndex, size_t offset, size_t size)
{
    assert(dynamicIndex < buffers_.size());
    assert(size > 0);

    auto freeOffset = offset;
    auto freeSize = size;

    auto& ranges = freeRanges_[dynamicIndex];

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

    auto& buffer = buffers_[dynamicIndex];
    std::fill_n(buffer.data() + offset, size, std::byte{ 0 });
}

void ResourceManager::erase(Resource::Id id)
{
    auto& resource = get(id);

    {

        auto const& tag = resource.instance_tag();

        if (tag.dynamicDataIndex < buffers_.size()) {
            freeDynamicBuffer(tag.dynamicDataIndex, resource.dynamicDataOffset(), resource.dynamicDataSize());
        }

        resource.dynamicOffset_ = std::numeric_limits<size_t>::max();
        resource.dynamicSize_ = 0;
    }

    resource.~Resource();

    auto& [size, version, itemIndex, staticIndex, dynamicIndex] = resources_[id.index()];

    auto& freeItems = freeItems_[staticIndex];
    auto& storage = storages_[staticIndex];

    auto freeOffset = size * itemIndex;
    auto freeSize = size;

    std::fill_n(storage.data() + freeOffset, freeSize, std::byte{ 0 });

    freeItems.push_back(itemIndex);

    size = 0;
    version++;
    itemIndex = std::numeric_limits<uint32_t>::max();
    staticIndex = std::numeric_limits<uint16_t>::max();
    dynamicIndex = std::numeric_limits<uint16_t>::max();
}

auto ResourceManager::isValid(Resource::Id id) const -> bool
{
    return id.index() < resources_.size() && resources_[id.index()].version == id.version();
}

auto ResourceManager::isValid(Resource const& resource) const -> bool
{
    return isValid(resource.id());
}

auto ResourceManager::get(Resource::Id id) const -> Resource const&
{
    assert(isValid(id));
    auto const& resource = resources_[id.index()];
    return *reinterpret_cast<Resource const*>(storages_[resource.static_index].data() + resource.item * resource.size);
}

auto ResourceManager::get(Resource::Id id) -> Resource&
{
    return const_cast<Resource&>(std::as_const(*this).get(id));
}

auto ResourceManager::getDynamicData(Resource::Id id) -> std::byte*
{
    auto& resource = get(id);
    auto const& tag = resource.instance_tag();

    return buffers_[tag.dynamicDataIndex].data() + resource.dynamicDataOffset();
}

auto ResourceManager::getDynamicData(Resource::Id id) const -> std::byte const*
{
    auto& resource = get(id);
    auto const& tag = resource.instance_tag();

    return buffers_[tag.dynamicDataIndex].data() + resource.dynamicDataOffset();
}

ResourceManager::~ResourceManager()
{
    for (auto index = size_t{ 0 }, count = resources_.size(); index < count; index++) {
        auto&& r = resources_[index];

        if (r.size == 0)
            continue;

        erase(Resource::Id{ static_cast<uint32_t>(index), r.version });
    }
}
}