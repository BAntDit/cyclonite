//
// Created by anton on 7/26/25.
//

#include "vkRenderPass.h"
#include "gfx/resourceManager.h"

#if defined(GFX_DRIVER_VULKAN)
namespace cyclonite::gfx::vulkan {
RenderPass::RenderPass(core::ResourceManagerBase* resourceManager,
                       core::ResourceId resourceId,
                       core::ResourceRef deviceRef)
  : core::ResourceBase{ resourceManager, resourceId, true }
  , renderPass_{ deviceRef.as<type_traits::platform_implementation_t<gfx::Device>>().handle(), vkDestroyRenderPass }
{
    auto renderPassCreateInfo = VkRenderPassCreateInfo{};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
}
}
#endif
