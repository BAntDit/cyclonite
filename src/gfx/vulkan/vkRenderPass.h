//
// Created by anton on 7/26/25.
//

#ifndef CYCLONITE_VKRENDERPASS_H
#define CYCLONITE_VKRENDERPASS_H

#include "core/resourceBase.h"
#include "gfx/common.h"
#include "gfx/config.h"
#include "handle.h"
#include <array>

#if defined(GFX_DRIVER_VULKAN)
namespace cyclonite::gfx::vulkan {
class RenderPass : public core::ResourceBase
{
public:
    RenderPass(core::ResourceManagerBase* resourceManager,
               core::ResourceId resourceId,
               core::ResourceRef deviceRef,
               core::ResourceRef depthStencilRef,
               std::array<core::ResourceRef, compile_time_config_t::max_color_attachment_count_v> colorAttachmentRefs);

    // begin

    // end

private:
    Handle<VkRenderPass> renderPass_;
};
}
#endif // GFX_DRIVER_VULKAN
#endif // CYCLONITE_VKRENDERPASS_H
