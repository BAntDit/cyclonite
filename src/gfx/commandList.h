//
// Created by anton on 9/28/25.
//

#ifndef CYCLONITE_GFX_COMMAND_LIST_H
#define CYCLONITE_GFX_COMMAND_LIST_H

#include "interfaces/commandListInterface.h"
#if defined(GFX_DRIVER_VULKAN)
#include "vulkan/vkCommandList.h"
#elif defined(GFX_DRIVER_D3D12)
// TODO:: d3d12 not implemented
#else
// TODO:: null api not implemented
#endif

namespace cyclonite::gfx {
#if defined(GFX_DRIVER_VULKAN)
using CommandList = interfaces::CommandListInterface<vulkan::CommandList>;
#elif defined(GFX_DRIVER_D3D12)
// TODO:: impl
#else
// TODO:: impl
#endif
}

#endif // CYCLONITE_GFX_COMMAND_LIST_H