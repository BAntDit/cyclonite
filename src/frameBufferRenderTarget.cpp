//
// Created by bantdit on 1/7/20.
//

#include "frameBufferRenderTarget.h"

namespace cyclonite {
FrameBufferRenderTarget::FrameBufferRenderTarget(vulkan::Device& device,
                                                 VkRenderPass vkRenderPass,
                                                 uint32_t width,
                                                 uint32_t height,
                                                 size_t bufferCount,
                                                 framebuffer_attachment_t const& depthStencil)
  : BaseRenderTarget(width, height, 0, true)
{
    for (auto i = size_t{ 0 }; i < bufferCount; i++) {
        frameBuffers_.emplace_back(
          device,
          vkRenderPass,
          width,
          height,
          getImageView(device,
                       width,
                       height,
                       depthStencil,
                       VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
                       VK_IMAGE_ASPECT_DEPTH_BIT));
    }
}

auto FrameBufferRenderTarget::swapBuffers(vulkan::Device const& device, uint32_t currentFrameImageIndex) -> uint32_t
{
    (void)device;
    (void)currentFrameImageIndex;

    throw std::runtime_error("not implemented yet");
}
}
