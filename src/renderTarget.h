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
                 std::array<VkFormat, count> surfaceFormats,
                 std::enable_if_t<(count > 0), void*> = nullptr);

    RenderTarget(RenderTarget const&) = delete;

    RenderTarget(RenderTarget&&) = default;

    ~RenderTarget() = default;

    auto operator=(RenderTarget const&) -> RenderTarget& = delete;

    auto operator=(RenderTarget &&) -> RenderTarget& = default;

    [[nodiscard]] auto width() const -> uint32_t { return extent_.width; }

    [[nodiscard]] auto height() const -> uint32_t { return extent_.height; }

    [[nodiscard]] auto colorAttachmentCount() const -> uint8_t { return colorAttachmentCount_; }

    [[nodiscard]] auto swapChainLength() const -> size_t { return swapChainLength_; }

    [[nodiscard]] auto currentChainIndex() const -> size_t { return currentChainIndex_; }

    [[nodiscard]] auto getColorAttachment(uint8_t attachmentIndex) const -> vulkan::ImageView const&;

    [[nodiscard]] auto getColorAttachment(RenderTargetOutputSemantic semantic) const -> vulkan::ImageView const&;

    [[nodiscard]] auto getDepthAttachment() const -> vulkan::ImageView const&;

    [[nodiscard]] auto hasAttachment(uint8_t attachmentIndex) const -> bool;

    [[nodiscard]] auto hasAttachment(RenderTargetOutputSemantic semantic) const -> bool;

    [[nodiscard]] auto hasDepth() const -> bool { return hasDepth_; }

private:
    VkExtent2D extent_;
    uint8_t colorAttachmentCount_;
    size_t swapChainLength_;
    size_t currentChainIndex_;
    std::optional<Surface> surface_;
    vulkan::Handle<VkSwapchainKHR> vkSwapChain_;
    std::vector<vulkan::FrameBuffer> frameBuffers_;
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
  , currentChainIndex_{ 0 }
  , surface_{}
  , vkSwapChain_{ device.handle(), vkDestroySwapchainKHR }
  , frameBuffers_{}
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
                           std::array<VkFormat, count> surfaceFormats,
                           std::enable_if_t<(count > 0), void*>)
  : extent_{}
  , colorAttachmentCount_{ count }
  , swapChainLength_{ swapChainLength }
  , currentChainIndex_{ 0 }
  , surface_{}
  , vkSwapChain_{ device.handle(), vkDestroySwapchainKHR }
  , frameBuffers_{}
  , outputSemantics_{}
  , hasDepth_{ true }
{
    // TODO::
    (void)vkRenderPass;
    (void)width;
    (void)height;
    (void)surfaceFormats;

    throw std::runtime_error("not implemented yet");
}
}

#endif // CYCLONITE_RENDERTARGET_H
