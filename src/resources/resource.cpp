//
// Created by anton on 5/25/22.
//

#include "resourceManager.h"
#include "resource.h"
#include <limits>

namespace cyclonite::resources {
std::atomic_uint16_t Resource::ResourceTag::_lastTagIndex{ std::numeric_limits<uint16_t>::max() };
Resource::ResourceTag Resource::tag{};

Resource::Id::Id() noexcept
  : id_{ 0 }
{}

Resource::Id::Id(uint64_t id) noexcept
  : id_{ id }
{}

Resource::Id::Id(uint32_t index, uint32_t version) noexcept
  : id_{ static_cast<uint64_t>(index) | static_cast<uint64_t>(version) << 32UL }
{}

Resource::ResourceTag::ResourceTag() noexcept
  : staticDataIndex{ std::numeric_limits<uint16_t>::max() }
  , dynamicDataIndex{ std::numeric_limits<uint16_t>::max() }
{}

Resource::Resource() noexcept
  : id_{}
  , resourceManager_{ nullptr }
  , dynamicOffset_{ std::numeric_limits<size_t>::max() }
  , dynamicSize_{ 0 }
  , state_{ ResourceState::UNLOADED }
{}

Resource::Resource(size_t dynamicSize) noexcept
  : id_{}
  , resourceManager_{ nullptr }
  , dynamicOffset_{ std::numeric_limits<size_t>::max() }
  , dynamicSize_{ dynamicSize }
  , state_{ ResourceState::UNLOADED }
{}

auto Resource::dynamicData() -> std::byte*
{
    return resourceManager_->getDynamicData(id_);
}
}
