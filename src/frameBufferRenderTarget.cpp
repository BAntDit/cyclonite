//
// Created by bantdit on 1/7/20.
//

#include "frameBufferRenderTarget.h"

namespace cyclonite {
FrameBufferRenderTarget::FrameBufferRenderTarget(vulkan::Device& device,
                                                 VkRenderPass vkRenderPass,
                                                 uint32_t width,
                                                 uint32_t height,
                                                 framebuffer_attachment_t const& depthStencil)
  : BaseRenderTarget(width, height, 0, true)
  , accessSemaphores_{}
{
    frameBuffers_.reserve(1);
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

    for (auto i = size_t{0}, count = accessSemaphores_.size(); i < count; i++) {
        accessSemaphores_[i] = vulkan::Handle<VkSemaphore>{ device.handle(), vkDestroySemaphore };
    }
}

auto FrameBufferRenderTarget::wait() const -> VkSemaphore
{
    return static_cast<VkSemaphore>(accessSemaphores_[0]);
}

auto FrameBufferRenderTarget::signal() const -> VkSemaphore
{
    return static_cast<VkSemaphore>(accessSemaphores_[1]);
}

auto FrameBufferRenderTarget::swapBuffers(vulkan::Device const& device, uint32_t currentFrameImageIndex) -> uint32_t
{
    (void)device;
    (void)currentFrameImageIndex;

    std::swap(accessSemaphores_[0], accessSemaphores_[1]);
}
}
