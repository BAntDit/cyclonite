//
// Created by anton on 6/12/25.
//

#ifndef GFX_INSTANCE_H
#define GFX_INSTANCE_H

#include "interfaces/instanceInterface.h"
#if defined(GFX_DRIVER_VULKAN)
#include "vulkan/vkInstance.h"
#elif defined(GFX_DRIVER_D3D12)
// TODO:: d3d12 not implemented
#else
// TODO:: null api not implemented
#endif

namespace cyclonite::gfx {
#if defined(GFX_DRIVER_VULKAN)
using Instance = interfaces::InstanceInterface<vulkan::Instance>;
#elif defined(GFX_DRIVER_D3D12)
// TODO:: impl
#else
// TODO:: impl
#endif
}

#endif // GFX_INSTANCE_H
