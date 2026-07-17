//
// Created by anton on 11/23/25.
//

#ifndef CYCLONITE_SHADER_RESOURCE_VIEW_H
#define CYCLONITE_SHADER_RESOURCE_VIEW_H

#include "interfaces/shaderResourceViewInterface.h"
#if defined(GFX_DRIVER_VULKAN)
#include "vulkan/vkShaderResourceView.h"
#elif defined(GFX_DRIVER_D3D12)
// TODO:: d3d12 not implemented
#else
// TODO:: null api not implemented
#endif

namespace cyclonite::gfx {
#if defined(GFX_DRIVER_VULKAN)
using ShaderResourceView = interfaces::ShaderResourceViewInterface<vulkan::ShaderResourceView>;
#elif defined(GFX_DRIVER_D3D12)
// TODO:: impl
#else
// TODO:: impl
#endif
}

#endif // CYCLONITE_SHADER_RESOURCE_VIEW_H