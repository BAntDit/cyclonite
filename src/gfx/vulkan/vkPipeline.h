//
// Created by anton on 11/2/25.
//

#ifndef CYCLONITE_GFX_VK_PIPELINE_H
#define CYCLONITE_GFX_VK_PIPELINE_H

#include "core/resourceBase.h"
#include "core/resourceSharedRef.h"
#include "gfx/common.h"
#include "gfx/config.h"
#include "handle.h"
#include <array>

#if defined(GFX_DRIVER_VULKAN)
namespace cyclonite::gfx::vulkan {
class Pipeline : public core::ResourceBase
{
public:
    Pipeline(core::ResourceManagerBase* resourceManager,
             core::ResourceId resourceId,
             core::ResourceSharedRef deviceRef,
             PipelineType type,
             PipelineCreationFlagBits creationFlags,
             std::array<core::ResourceSharedRef, config_t::max_shader_stage_count_v> const& shaders);

private:
    void initGraphicsPipeline(PipelineCreationFlagBits creationFlags,
                              std::array<core::ResourceSharedRef, config_t::max_shader_stage_count_v> const& shaders);

private:
    Handle<VkPipeline> vkPipeline_;
};
}
#endif // GFX_DRIVER_VULKAN
#endif // CYCLONITE_GFX_VK_PIPELINE_H