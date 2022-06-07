//
// Created by anton on 5/25/22.
//

#include "resource.h"
#include "resourceManager.h"
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>
#include <fstream>
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

Resource::Resource(ResourceManager* resourceManager) noexcept
  : id_{}
  , resourceManager_{ resourceManager }
  , dynamicOffset_{ std::numeric_limits<size_t>::max() }
  , dynamicSize_{ 0 }
  , state_{ ResourceState::UNLOADED }
{}

Resource::Resource(ResourceManager* resourceManager, size_t dynamicSize) noexcept
  : id_{}
  , resourceManager_{ resourceManager }
  , dynamicOffset_{ std::numeric_limits<size_t>::max() }
  , dynamicSize_{ dynamicSize }
  , state_{ ResourceState::UNLOADED }
{}

auto Resource::dynamicData() -> std::byte*
{
    return resourceManager_->getDynamicData(id_);
}

void Resource::load(std::filesystem::path const& path)
{
    std::ifstream file{};
    file.exceptions(std::ios::failbit);
    file.open(path.string());
    file.exceptions(std::ios::badbit);

    load(file);
}

void Resource::load(void const* data, size_t size)
{
    using source_array_t = boost::iostreams::array_source;
    boost::iostreams::stream<source_array_t> stream(reinterpret_cast<char const*>(data), size);

    load(stream);
}

void Resource::load(std::istream& stream)
{
    (void)stream;
    state_ = ResourceState::COMPLETE;
}

void Resource::handleDynamicBufferRealloc() {}
}
