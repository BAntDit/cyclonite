//
// Created by anton on 11/24/25.
//

#ifndef CYCLONITE_DESCRIPTOR_UPDATE_DATA_H
#define CYCLONITE_DESCRIPTOR_UPDATE_DATA_H

#include "common.h"
#include "core/resourceSharedRef.h"

namespace cyclonite::gfx {
struct DescriptorWriteData
{
    core::ResourceSharedRef resource;
    uint32_t binding;
    uint32_t element;
    std::variant<ResourceDescription, BufferResourceDescription, TextureResourceDescription> desc;
};

struct DescriptorCopyData
{
    uint32_t srcBinding;
    uint32_t srcElement;
    uint32_t dstBinding;
    uint32_t dstElement;
};
}

#endif // CYCLONITE_DESCRIPTOR_UPDATE_DATA_H