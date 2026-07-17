//
// Created by anton on 11/9/25.
//

#ifndef CYCLONITE_PIPELINE_BINDING_SCHEMA_H
#define CYCLONITE_PIPELINE_BINDING_SCHEMA_H

#include "interfaces/pipelineBindingSchemaInterface.h"
#if defined(GFX_DRIVER_VULKAN)
#include "vulkan/vkPipelineBindingSchema.h"
#elif defined(GFX_DRIVER_D3D12)
// TODO:: d3d12 not implemented
#else
// TODO:: null api not implemented
#endif

namespace cyclonite::gfx {
#if defined(GFX_DRIVER_VULKAN)
using PipelineBindingSchema = interfaces::PipelineBindingSchemaInterface<vulkan::PipelineBindingSchema>;
#elif defined(GFX_DRIVER_D3D12)
// TODO:: impl
#else
// TODO:: impl
#endif
}
#endif // CYCLONITE_PIPELINE_BINDING_SCHEMA_H
