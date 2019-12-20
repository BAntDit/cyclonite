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

template<bool hasDepth, size_t colorOutputCount>
class RenderTarget
{
public:
    // template<typename DepthStencilAttachment, typename... ColorAttachments>
    RenderTarget(VkRenderPass vkRenderPass);

    /* template<typename DepthStencilOutputDescription, typename ColorOutputDescription, size_t modeCandidateCount = 2>
    RenderTarget(
      vulkan::Device const& device,
      VkRenderPass vkRenderPass,
      Surface& surface,
      DepthStencilOutputDescription depthStencilOutputDescription,
      ColorOutputDescription colorOutputDescription = output_t<
        easy_mp::type_list<output_format_candidate_t<VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR>>,
        RenderTargetOutputSemantic::DEFAULT>{},
      std::array<VkPresentModeKHR, modeCandidateCount> const& presentModeCandidates = { VK_PRESENT_MODE_MAILBOX_KHR,
                                                                                        VK_PRESENT_MODE_FIFO_KHR },
      VkCompositeAlphaFlagBitsKHR vkCompositeAlphaFlags = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR); */

    using frame_buffer_t =
      std::conditional_t<hasDepth, vulkan::FrameBuffer<colorOutputCount + 1>, vulkan::FrameBuffer<colorOutputCount>>;

    template<bool Check = hasDepth && (colorOutputCount > 0)>
    RenderTarget(vulkan::Device const& device,
                 VkRenderPass vkRenderPass,
                 Surface& surface,
                 vulkan::Handle<VkSwapchainKHR> swapChain,
                 VkFormat depthFormat,
                 std::array<VkFormat, colorOutputCount> surfaceFormat,
                 std::enable_if_t<Check, void*> = nullptr);

    template<bool Check = hasDepth && (colorOutputCount > 0)>
    RenderTarget(vulkan::Device const& device,
                 VkRenderPass vkRenderPass,
                 Surface& surface,
                 vulkan::Handle<VkSwapchainKHR> swapChain,
                 std::array<VkFormat, colorOutputCount> surfaceFormat,
                 std::enable_if_t<Check, void*> = nullptr);

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

private:
    VkExtent2D extent_;
    uint8_t colorAttachmentCount_;
    size_t swapChainLength_;
    size_t currentChainIndex_;
    std::optional<Surface> surface_;
    vulkan::Handle<VkSwapchainKHR> vkSwapChain_;
    std::vector<frame_buffer_t> frameBuffers_;
};

RenderTarget::RenderTarget(vulkan::Device const& device, VkRenderPass vkRenderPass, Surface& surface)
  : extent_{}
  , colorAttachmentCount_{ 1 }
  , swapChainLength_{ 0 }
  , currentChainIndex_{ 0 }
  , surface_{ std::move(surface) }
  , vkSwapChain_{ device.handle(), vkDestroySwapchainKHR }
  , frameBuffers_{}
{
    uint32_t imageCount = 0;
    vkGetSwapchainImagesKHR(device.handle(), static_cast<VkSwapchainKHR>(vkSwapChain_), &imageCount, nullptr);

    swapChainLength_ = imageCount;

    std::vector<VkImage> vkImages(swapChainLength_, VK_NULL_HANDLE);

    vkGetSwapchainImagesKHR(device.handle(), static_cast<VkSwapchainKHR>(vkSwapChain_), &imageCount, vkImages.data());

    frameBuffers_.reserve(swapChainLength_);

    for (auto vkImage : vkImages) {
        frameBuffers_.emplace_back(
          device,
          vkRenderPass,
          extent_.width,
          extent_.height,
          std::array<vulkan::ImageView, 1>{ vulkan::ImageView{
            device, std::make_shared<vulkan::Image>(vkImage, extent_.width, extent_.height, surfaceFormat) } });
    }
}
}

#endif // CYCLONITE_RENDERTARGET_H
