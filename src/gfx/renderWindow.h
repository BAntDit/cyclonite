//
// Created by anton on 6/11/25.
//

#ifndef GFX_RENDER_WINDOW_H
#define GFX_RENDER_WINDOW_H

#include "interfaces/renderWindowInterface.h"
#if defined(GFX_DRIVER_VULKAN)
#include "vulkan/vkRenderWindow.h"
#elif defined(GFX_DRIVER_D3D12)
// TODO:: d3d12 not implemented
#else
// TODO:: null api not implemented
#endif

namespace cyclonite::gfx {
#if defined(GFX_DRIVER_VULKAN)
using RenderWindow = interfaces::RenderWindowInterface<vulkan::RenderWindow>;
#elif defined(GFX_DRIVER_D3D12)
// TODO:: impl
#else
// TODO:: impl
#endif
}

#endif // GFX_RENDER_WINDOW_H
