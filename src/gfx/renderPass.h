//
// Created by anton on 8/27/25.
//

#ifndef CYCLONITE_RENDERPASS_H
#define CYCLONITE_RENDERPASS_H

#include "interfaces/renderPassInterface.h"
#if defined(GFX_DRIVER_VULKAN)
#include "vulkan/vkRenderPass.h"
#elif defined(GFX_DRIVER_D3D12)
// TODO:: d3d12 not implemented
#else
// TODO:: null api not implemented
#endif

namespace cyclonite::gfx {
#if defined(GFX_DRIVER_VULKAN)
using RenderPass = interfaces::RenderPassInterface<vulkan::RenderPass>;
#elif defined(GFX_DRIVER_D3D12)
// TODO:: impl
#else
// TODO:: impl
#endif
}

#endif // CYCLONITE_RENDERPASS_H
