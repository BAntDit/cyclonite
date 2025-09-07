//
// Created by anton on 9/7/25.
//

#ifndef CYCLONITE_GFX_COMMANDPOOL_H
#define CYCLONITE_GFX_COMMANDPOOL_H

#include "interfaces/commandPoolInterface.h"
#if defined(GFX_DRIVER_VULKAN)
#include "vulkan/vkCommandPool.h"
#elif defined(GFX_DRIVER_D3D12)
// TODO:: d3d12 not implemented
#else
// TODO:: null api not implemented
#endif

namespace cyclonite::gfx {
#if defined(GFX_DRIVER_VULKAN)
using CommandPool = interfaces::CommandPoolInterface<vulkan::CommandPool>;
#elif defined(GFX_DRIVER_D3D12)
// TODO:: impl
#else
// TODO:: impl
#endif
}

#endif // CYCLONITE_GFX_COMMANDPOOL_H
