//
// Created by anton on 6/16/25.
//

#ifndef GFX_DEVICE_H
#define GFX_DEVICE_H

#include "interfaces/deviceInterface.h"
#if defined(GFX_DRIVER_VULKAN)
#include "vulkan/vkDevice.h"
#elif defined(GFX_DRIVER_D3D12)
// TODO:: d3d12 not implemented
#else
// TODO:: null api not implemented
#endif

namespace cyclonite::gfx {
#if defined(GFX_DRIVER_VULKAN)
using Device = interfaces::DeviceInterface<vulkan::Device>;
#elif defined(GFX_DRIVER_D3D12)
// TODO:: impl
#else
// TODO:: impl
#endif
}

#endif // GFX_DEVICE_H
