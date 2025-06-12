//
// Created by anton on 6/11/25.
//

#ifndef GFX_SURFACE_H
#define GFX_SURFACE_H

#include "./interfaces/surfaceInterface.h"
#if defined(GFX_DRIVER_VULKAN)
#include "./vulkan/vkSurface.h"
#elif defined(GFX_DRIVER_D3D12)
// TODO:: d3d12 not implemented
#else
// TODO:: null api not implemented
#endif

namespace cyclonite::gfx {
#if defined(GFX_DRIVER_VULKAN)
using Surface = interfaces::SurfaceInterface<vulkan::Surface>;
#elif defined(GFX_DRIVER_D3D12)
// TODO:: impl
#else
// TODO:: impl
#endif
}

#endif // GFX_SURFACE_H
