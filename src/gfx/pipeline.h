//
// Created by anton on 11/10/25.
//

#ifndef CYCLONITE_GFX_PIPELINE_H
#define CYCLONITE_GFX_PIPELINE_H

#include "interfaces/pipelineInterface.h"
#if defined(GFX_DRIVER_VULKAN)
#include "vulkan/vkPipeline.h"
#elif defined(GFX_DRIVER_D3D12)
// TODO:: d3d12 not implemented
#else
// TODO:: null api not implemented
#endif

namespace cyclonite::gfx {
#if defined(GFX_DRIVER_VULKAN)
using Pipeline = interfaces::PipelineInterface<vulkan::Pipeline>;
#elif defined(GFX_DRIVER_D3D12)
// TODO:: impl
#else
// TODO:: impl
#endif
}
#endif // CYCLONITE_GFX_PIPELINE_H
