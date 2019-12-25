//
// Created by bantdit on 11/3/19.
//

#ifndef CYCLONITE_RENDERTARGET_H
#define CYCLONITE_RENDERTARGET_H

#include "surface.h"
#include "vulkan/frameBuffer.h"

namespace cyclonite {
enum class RenderTargetOutputSemantic
{
    UNDEFINED = 0,
    DEFAULT = 1,
    LINEAR_HDR_COLOR = 2,
    MIN_VALUE = UNDEFINED,
    MAX_VALUE = LINEAR_HDR_COLOR,
    COUNT = MAX_VALUE + 1
};

// TODO:: split SurfaceRenderTarget and FrameBufferRenderTarget
class RenderTarget
{
public:
    RenderTarget(vulkan::Device& device,
                 VkRenderPass vkRenderPass,
                 Surface& surface,
                 vulkan::Handle<VkSwapchainKHR>& vkSwapChain,
                 VkFormat depthFormat,
                 VkFormat surfaceFormat,
                 RenderTargetOutputSemantic outputSemantic);

    RenderTarget(vulkan::Device& device,
                 VkRenderPass vkRenderPass,
                 Surface& surface,
                 vulkan::Handle<VkSwapchainKHR> vkSwapChain,
                 VkFormat surfaceFormat,
                 RenderTargetOutputSemantic outputSemantic);

    template<size_t count>
    RenderTarget(vulkan::Device& device,
                 VkRenderPass vkRenderPass,
                 uint32_t swapChainLength,
                 uint32_t width,
                 uint32_t height,
                 VkFormat depthFormat,
                 std::array<std::pair<VkFormat, RenderTargetOutputSemantic>, count> colorOutputs);

    template<size_t count>
    RenderTarget(vulkan::Device& device,
                 VkRenderPass vkRenderPass,
                 uint32_t swapChainLength,
                 uint32_t width,
                 uint32_t height,
                 std::array<std::pair<VkFormat, RenderTargetOutputSemantic>, count> colorOutputs,
                 std::enable_if_t<(count > 0), void*> = nullptr);

    RenderTarget(RenderTarget const&) = delete;

    RenderTarget(RenderTarget&&) = default;

    ~RenderTarget() = default;

    auto operator=(RenderTarget const&) -> RenderTarget& = delete;

    auto operator=(RenderTarget &&) -> RenderTarget& = default;

    void swapBuffers(vulkan::Device const& device, VkSemaphore passFinishedSemaphore);

    [[nodiscard]] auto width() const -> uint32_t { return extent_.width; }

    [[nodiscard]] auto height() const -> uint32_t { return extent_.height; }

    [[nodiscard]] auto colorAttachmentCount() const -> uint8_t { return colorAttachmentCount_; }

    [[nodiscard]] auto swapChainLength() const -> size_t { return swapChainLength_; }

    [[nodiscard]] auto frontBufferIndex() const -> size_t { return frontBufferIndex_; }

    [[nodiscard]] auto getBackBufferIndex(vulkan::Device const& device) -> size_t;

    [[nodiscard]] auto getColorAttachment(uint8_t attachmentIndex) const -> vulkan::ImageView const&;

    [[nodiscard]] auto getColorAttachment(RenderTargetOutputSemantic semantic) const -> vulkan::ImageView const&;

    [[nodiscard]] auto getDepthAttachment() const -> vulkan::ImageView const&;

    [[nodiscard]] auto hasAttachment(uint8_t attachmentIndex) const -> bool;

    [[nodiscard]] auto hasAttachment(RenderTargetOutputSemantic semantic) const -> bool;

    [[nodiscard]] auto hasDepth() const -> bool { return hasDepth_; }

    [[nodiscard]] auto frameBuffers() const -> std::vector<vulkan::FrameBuffer> const& { return frameBuffers_; }

private:
    VkExtent2D extent_;
    uint8_t colorAttachmentCount_;
    size_t swapChainLength_;
    size_t frontBufferIndex_;
    size_t backBufferIndex_;
    std::optional<Surface> surface_;
    vulkan::Handle<VkSwapchainKHR> vkSwapChain_;
    std::vector<vulkan::FrameBuffer> frameBuffers_;
    std::vector<vulkan::Handle<VkSemaphore>> imageAvailableSemaphores_;
    std::unordered_map<RenderTargetOutputSemantic, size_t> outputSemantics_;
    bool hasDepth_;
};

template<size_t count>
RenderTarget::RenderTarget(vulkan::Device& device,
                           VkRenderPass vkRenderPass,
                           uint32_t swapChainLength,
                           uint32_t width,
                           uint32_t height,
                           VkFormat depthFormat,
                           std::array<std::pair<VkFormat, RenderTargetOutputSemantic>, count> colorOutputs)
  : extent_{}
  , colorAttachmentCount_{ count }
  , swapChainLength_{ swapChainLength }
  , frontBufferIndex_{ 0 }
  , backBufferIndex_{ 0 }
  , surface_{}
  , vkSwapChain_{ device.handle(), vkDestroySwapchainKHR }
  , frameBuffers_{}
  , imageAvailableSemaphores_{}
  , outputSemantics_{}
  , hasDepth_{ true }
{
    // TODO::
    (void)vkRenderPass;
    (void)width;
    (void)height;
    (void)depthFormat;
    (void)colorOutputs;

    throw std::runtime_error("not implemented yet");
}

template<size_t count>
RenderTarget::RenderTarget(vulkan::Device& device,
                           VkRenderPass vkRenderPass,
                           uint32_t swapChainLength,
                           uint32_t width,
                           uint32_t height,
                           std::array<std::pair<VkFormat, RenderTargetOutputSemantic>, count> colorOutputs,
                           std::enable_if_t<(count > 0), void*>)
  : extent_{}
  , colorAttachmentCount_{ count }
  , swapChainLength_{ swapChainLength }
  , frontBufferIndex_{ 0 }
  , backBufferIndex_{ 0 }
  , surface_{}
  , vkSwapChain_{ device.handle(), vkDestroySwapchainKHR }
  , frameBuffers_{}
  , imageAvailableSemaphores_{}
  , outputSemantics_{}
  , hasDepth_{ true }
{
    // TODO::
    (void)vkRenderPass;
    (void)width;
    (void)height;
    (void)colorOutputs;

    throw std::runtime_error("not implemented yet");
}
}

#endif // CYCLONITE_RENDERTARGET_H
