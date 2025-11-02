//
// Created by anton on 11/2/25.
//

#include "vkPipeline.h"
#include "gfx/device.h"

#if defined(GFX_DRIVER_VULKAN)
namespace cyclonite::gfx::vulkan {
Pipeline::Pipeline(core::ResourceManagerBase* resourceManager,
                   core::ResourceId resourceId,
                   core::ResourceSharedRef deviceRef,
                   PipelineType type,
                   PipelineCreationFlagBits creationFlags)
  : core::ResourceBase{ resourceManager, resourceId, false }
  , vkPipeline_{ deviceRef.as<type_traits::platform_implementation_t<gfx::Device>>().handle(), vkDestroyPipeline }
{
    if (type == PipelineType::Graphics) {
        initGraphicsPipeline(creationFlags);
    } else if (type == PipelineType::Compute) {
        // TODO:: ...
    }
}

void Pipeline::initGraphicsPipeline(PipelineCreationFlagBits creationFlags)
{
    auto pipelineInfo = VkGraphicsPipelineCreateInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.flags = creationFlags.cast_to<VkPipelineCreateFlags>();
}
}
#endif // GFX_DRIVER_VULKAN
