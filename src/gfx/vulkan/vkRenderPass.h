//
// Created by anton on 7/26/25.
//

#ifndef CYCLONITE_VKRENDERPASS_H
#define CYCLONITE_VKRENDERPASS_H

#include "core/resourceBase.h"
#include "core/resourceRef.h"
#include "gfx/common.h"
#include "gfx/config.h"
#include "handle.h"
#include <array>
#include <variant>

#if defined(GFX_DRIVER_VULKAN)
namespace cyclonite::gfx::vulkan {
class RenderPass : public core::ResourceBase
{
public:
    RenderPass(core::ResourceManagerBase* resourceManager,
               core::ResourceId resourceId,
               core::ResourceRef deviceRef,
               core::ResourceRef depthStencilRef,
               std::array<core::ResourceRef, compile_time_config_t::max_color_attachment_count_v> colorAttachmentRefs,
               std::array<std::pair<uint16_t, uint16_t>, compile_time_config_t::max_color_attachment_count_v>
                 colorAttachmentSubresDescs,
               uint32_t width,
               uint32_t height);

    RenderPass(core::ResourceManagerBase* resourceManager,
               core::ResourceId resourceId,
               core::ResourceRef deviceRef,
               core::ResourceRef renderWindowRef,
               gfx::Format depthStencilFormat = gfx::Format::UNDEFINED);

    // begin

    // end

    using core::ResourceBase::resourceBase;

private:
    using render_windows_ref = core::ResourceRef;
    using depth_stencil_ref = core::ResourceRef;
    using color_attachment_ref = core::ResourceRef;
    using render_targets_t =
      std::variant<render_windows_ref,
                   std::pair<depth_stencil_ref,
                             std::array<color_attachment_ref, compile_time_config_t::max_color_attachment_count_v>>>;

    core::ResourceRef deviceRef_;
    render_targets_t renderTargets_;
    Handle<VkRenderPass> vkRenderPass_;
    std::array<Handle<VkFramebuffer>, compile_time_config_t::max_swapchain_length_v> vkFrameBuffers_;
    uint32_t bufferCount_;
    uint32_t currentBufferIndex_;
};
}
#endif // GFX_DRIVER_VULKAN
#endif // CYCLONITE_VKRENDERPASS_H
