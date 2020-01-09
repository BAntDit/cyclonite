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
                            VkClearDepthStencilValue clearDepthStencilValue,
                            std::array<std::pair<VkFormat, RenderTargetOutputSemantic>, count> const& colorOutputs,
                            std::array<VkClearColorValue, count> const& clearColorValues);

    template<size_t count>
    FrameBufferRenderTarget(vulkan::Device& device,
                            VkRenderPass vkRenderPass,
                            uint32_t swapChainLength,
                            uint32_t width,
                            uint32_t height,
                            std::array<std::pair<VkFormat, RenderTargetOutputSemantic>, count> const& colorOutputs,
                            std::array<VkClearColorValue, count> const& clearColorValues);

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
  VkClearDepthStencilValue clearDepthStencilValue,
  std::array<std::pair<VkFormat, RenderTargetOutputSemantic>, count> const& colorOutputs,
  std::array<VkClearColorValue, count> const& clearColorValues)
  : BaseRenderTarget(width, height, clearDepthStencilValue, clearColorValues)
{
    swapChainLength_ = swapChainLength;

    (void)depthStencilFormat;
    (void)device;
    (void)vkRenderPass;
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
  std::array<std::pair<VkFormat, RenderTargetOutputSemantic>, count> const& colorOutputs,
  std::array<VkClearColorValue, count> const& clearColorValues)
  : BaseRenderTarget(width, height, clearColorValues)
{
    swapChainLength_ = swapChainLength;

    (void)device;
    (void)vkRenderPass;
    (void)colorOutputs;
}
}

#endif // CYCLONITE_FRAMEBUFFERRENDERTARGET_H
