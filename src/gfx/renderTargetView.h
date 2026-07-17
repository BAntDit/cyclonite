
#ifndef GFX_RTV_H
#define GFX_RTV_H

#include "interfaces/renderTargetViewInterface.h"
#if defined(GFX_DRIVER_VULKAN)
#include "vulkan/vkRenderTargetView.h"
#elif defined(GFX_DRIVER_D3D12)
// TODO:: d3d12 not implemented
#else
// TODO:: null api not implemented
#endif

namespace cyclonite::gfx {
#if defined(GFX_DRIVER_VULKAN)
using RenderTargetView = interfaces::RenderTargetViewInterface<vulkan::RenderTargetView>;
#elif defined(GFX_DRIVER_D3D12)
// TODO:: impl
#else
// TODO:: impl
#endif
}

#endif // GFX_RTV_H
