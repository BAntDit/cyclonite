//
// Created by anton on 11/16/25.
//

#ifndef CYCLONITE_GFX_DESCRIPTOR_SET_H
#define CYCLONITE_GFX_DESCRIPTOR_SET_H

#include "interfaces/descriptorSetInterface.h"
#if defined(GFX_DRIVER_VULKAN)
#include "vulkan/vkDescriptorSet.h"
#elif defined(GFX_DRIVER_D3D12)
// TODO:: d3d12 not implemented
#else
// TODO:: null api not implemented
#endif

namespace cyclonite::gfx {
#if defined(GFX_DRIVER_VULKAN)
using DescriptorSet = interfaces::DescriptorSetInterface<vulkan::DescriptorSet>;
#elif defined(GFX_DRIVER_D3D12)
// TODO:: impl
#else
// TODO:: impl
#endif
}

#endif // CYCLONITE_GFX_DESCRIPTOR_SET_H