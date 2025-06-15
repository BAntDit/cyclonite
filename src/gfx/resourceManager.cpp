//
// Created by anton on 6/15/25.
//

#include "resourceManager.h"
#include <cassert>

namespace cyclonite::gfx {
auto ResourceManager::alloc(uint16_t type) -> std::pair<uint32_t, uint32_t>
{
    assert(type < resource_meta_t::resource_type_count_v);

    auto blockIndex = std::numeric_limits<uint32_t>::max();

    if (storage_.freeIndices[type].empty()) {
        blockIndex = storage_.freeIndices[type].size();
        storage_.resources[type].emplace_back();
    } else {
        blockIndex = storage_.freeIndices[type].back();
        storage_.resources[type].pop_back();
    }
    assert(blockIndex < storage_.resources[type].size());

    auto headerIndex = std::numeric_limits<uint32_t>::max();
    if (!emptyHeaders_.empty()) {
        headerIndex = emptyHeaders_.size();
        emptyHeaders_.emplace_back();
    } else {
        headerIndex = emptyHeaders_.back();
        emptyHeaders_.pop_back();
    }
    assert(headerIndex < headers_.size());

    return std::pair{ headerIndex, blockIndex };
}

void ResourceManager::gc(bool clearAll /* = false*/)
{
    // TODO::
}

auto ResourceManager::isResourceValid(ResourceId id) const -> bool
{
    assert(id.index() < headers_.size());
    auto const& header = headers_[id.index()];

    return id.version() == header.version;
}

void ResourceManager::releaseResourceImmediate(ResourceId id)
{
    assert(isResourceValid(id));

    auto& header = headers_[id.index()];
    auto idx = header.index;
    auto type = header.type;

    header.deleter(storage_.resources[type][idx].bytes);

    storage_.freeIndices[type].push_back(idx);

    header.version++;
    header.type = std::numeric_limits<uint16_t>::max();
    header.index = std::numeric_limits<uint32_t>::max();
    header.deleter = [](void* ptr) -> void {};

    emptyHeaders_.push_back(id.index());
}

void ResourceManager::releaseResourceDeferred(ResourceId id)
{
    // TODO::
}

ResourceManager::~ResourceManager()
{
    gc(true);
}
}