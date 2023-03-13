//
// Created by anton on 3/11/23.
//

#ifndef CYCLONITE_SHADERRESOURCE_H
#define CYCLONITE_SHADERRESOURCE_H

#include "descriptorType.h"
#include <array>
#include <cstddef>
#include <cstdint>
#include <easy-mp/algorithm.h>

namespace cyclonite::render {
struct ShaderResourceHeader
{
    DescriptorType descriptorType;
};

struct ShaderResourceCommonDescriptor
{
    uint8_t set;
    uint8_t binding;
};

struct ShaderResourceSSBODescriptor : ShaderResourceCommonDescriptor
{
    size_t offset;
};

struct ShaderResourceUBODescriptor : ShaderResourceCommonDescriptor
{
    size_t offset;
    size_t size;
};

struct ShaderResource
{
    ShaderResourceHeader header;
    std::array<std::byte,
               easy_mp::max(sizeof(ShaderResourceCommonDescriptor),
                            sizeof(ShaderResourceSSBODescriptor),
                            sizeof(ShaderResourceUBODescriptor))>
      descriptor;
};
}

#endif // CYCLONITE_SHADERRESOURCE_H
