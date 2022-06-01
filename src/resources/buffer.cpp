//
// Created by anton on 5/31/22.
//

#include "buffer.h"

namespace cyclonite::resources {
Buffer::Buffer(size_t size) noexcept
  : Resource(size)
{}

void Buffer::load(std::istream& stream)
{
    state_ = ResourceState::LOADING;
    stream.read(reinterpret_cast<char*>(dynamicData()), static_cast<std::streamsize>(dynamicDataSize()));

    state_ = ResourceState::COMPLETE;
}
}