//
// Created by bantdit on 1/7/20.
//

#ifndef CYCLONITE_FRAMEBUFFERRENDERTARGET_H
#define CYCLONITE_FRAMEBUFFERRENDERTARGET_H

#include "baseRenderTarget.h"

namespace cyclonite {
class FrameBufferRenderTarget : public BaseRenderTarget
{
public:
    template<size_t count>
    FrameBufferRenderTarget(vulkan::Device& device,
                            VkRenderPass vkRenderPass,
                            uint32_t swapChainLength,
                            uint32_t width,
                            uint32_t height,
                            VkFormat depthStencilFormat,
                            std::array<std::pair<VkFormat, RenderTargetOutputSemantic>, count> const& colorOutputs);

    template<size_t count>
    FrameBufferRenderTarget(vulkan::Device& device,
                            VkRenderPass vkRenderPass,
                            uint32_t swapChainLength,
                            uint32_t width,
                            uint32_t height,
                            std::array<std::pair<VkFormat, RenderTargetOutputSemantic>, count> const& colorOutputs);

    FrameBufferRenderTarget(FrameBufferRenderTarget const&) = delete;

    FrameBufferRenderTarget(FrameBufferRenderTarget&&) = default;

    ~FrameBufferRenderTarget() = default;

    auto operator=(FrameBufferRenderTarget const&) -> FrameBufferRenderTarget& = delete;

    auto operator=(FrameBufferRenderTarget &&) -> FrameBufferRenderTarget& = default;

    void swapBuffers(vulkan::Device const& device, VkSemaphore passFinishedSemaphore);
};

template<size_t count>
FrameBufferRenderTarget::FrameBufferRenderTarget(
  vulkan::Device& device,
  VkRenderPass vkRenderPass,
  uint32_t swapChainLength,
  uint32_t width,
  uint32_t height,
  VkFormat depthStencilFormat,
  std::array<std::pair<VkFormat, RenderTargetOutputSemantic>, count> const& colorOutputs)
  : BaseRenderTarget(width, height)
{
    colorAttachmentCount_ = 2;

    hasDepthStencil_ = depthStencilFormat != VK_FORMAT_UNDEFINED;

    swapChainLength_ = swapChainLength;

    (void)device;
    (void)vkRenderPass;
    (void)width;
    (void)height;
    (void)colorOutputs;

    throw std::runtime_error("not implemented yet");
}

template<size_t count>
FrameBufferRenderTarget::FrameBufferRenderTarget(
  vulkan::Device& device,
  VkRenderPass vkRenderPass,
  uint32_t swapChainLength,
  uint32_t width,
  uint32_t height,
  std::array<std::pair<VkFormat, RenderTargetOutputSemantic>, count> const& colorOutputs)
  : FrameBufferRenderTarget(device, vkRenderPass, swapChainLength, width, height, VK_FORMAT_UNDEFINED, colorOutputs)
{}
}

#endif // CYCLONITE_FRAMEBUFFERRENDERTARGET_H
