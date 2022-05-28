//
// Created by anton on 5/25/22.
//

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
  : index{ std::numeric_limits<uint16_t>::max() }
  , isFixedSizePerItem{ false }
{}
}
