//
// Created by anton on 11/1/25.
//

#ifndef CYCLONITE_GFX_BUFFER_H
#define CYCLONITE_GFX_BUFFER_H

#include "interfaces/bufferInterface.h"
#if defined(GFX_DRIVER_VULKAN)
#include "vulkan/vkBuffer.h"
#elif defined(GFX_DRIVER_D3D12)
// TODO:: d3d12 not implemented
#else
// TODO:: null api not implemented
#endif

namespace cyclonite::gfx {
#if defined(GFX_DRIVER_VULKAN)
using Buffer = interfaces::BufferInterface<vulkan::Buffer>;
#elif defined(GFX_DRIVER_D3D12)
// TODO:: impl
#else
// TODO:: impl
#endif
}

#endif // CYCLONITE_GFX_BUFFER_H