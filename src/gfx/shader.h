//
// Created by anton on 11/2/25.
//

#ifndef CYCLONITE_GFX_SHADER_H
#define CYCLONITE_GFX_SHADER_H

#include "interfaces/shaderInterface.h"
#if defined(GFX_DRIVER_VULKAN)
#include "vulkan/vkShader.h"
#elif defined(GFX_DRIVER_D3D12)
// TODO:: d3d12 not implemented
#else
// TODO:: null api not implemented
#endif

namespace cyclonite::gfx {
#if defined(GFX_DRIVER_VULKAN)
using Shader = interfaces::ShaderInterface<vulkan::Shader>;
#elif defined(GFX_DRIVER_D3D12)
// TODO:: impl
#else
// TODO:: impl
#endif
}

#endif // CYCLONITE_GFX_SHADER_H
