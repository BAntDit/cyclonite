//
// Created by anton on 11/3/25.
//

#ifndef CYCLONITE_SHADER_STAGE_H
#define CYCLONITE_SHADER_STAGE_H

#include "interfaces/shaderStageInterface.h"
#if defined(GFX_DRIVER_VULKAN)
#include "vulkan/vkShaderStage.h"
#elif defined(GFX_DRIVER_D3D12)
// TODO:: d3d12 not implemented
#else
// TODO:: null api not implemented
#endif

namespace cyclonite::gfx
{
#if defined(GFX_DRIVER_VULKAN)
using ShaderStage = interfaces::ShaderStageInterface<vulkan::ShaderStage>;
#elif defined(GFX_DRIVER_D3D12)
// TODO:: impl
#else
// TODO:: impl
#endif
}

#endif //CYCLONITE_SHADER_STAGE_H