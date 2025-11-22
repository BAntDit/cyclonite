//
// Created by anton on 11/22/25.
//

#ifndef CYCLONITE_GFX_SAMPLER_H
#define CYCLONITE_GFX_SAMPLER_H

#include "interfaces/samplerInterface.h"
#if defined(GFX_DRIVER_VULKAN)
#include "vulkan/vkSampler.h"
#elif defined(GFX_DRIVER_D3D12)
// TODO:: d3d12 not implemented
#else
// TODO:: null api not implemented
#endif

namespace cyclonite::gfx {
#if defined(GFX_DRIVER_VULKAN)
using Sampler = interfaces::SamplerInterface<vulkan::Sampler>;
#elif defined(GFX_DRIVER_D3D12)
// TODO:: impl
#else
// TODO:: impl
#endif
}

#endif // CYCLONITE_GFX_SAMPLER_H