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
    using frame_buffer_t =
      std::conditional_t<hasDepth, vulkan::FrameBuffer<colorOutputCount + 1>, vulkan::FrameBuffer<colorOutputCount>>;

    template<bool Check = hasDepth && (colorOutputCount == 1)>
    RenderTarget(vulkan::Device& device,
                 VkRenderPass vkRenderPass,
                 Surface& surface,
                 vulkan::Handle<VkSwapchainKHR>& vkSwapChain,
                 VkFormat depthFormat,
                 std::array<VkFormat, colorOutputCount> surfaceFormats,
                 std::enable_if_t<Check, void*> = nullptr);

    template<bool Check = !hasDepth && (colorOutputCount == 1)>
    RenderTarget(vulkan::Device& device,
                 VkRenderPass vkRenderPass,
                 Surface& surface,
                 vulkan::Handle<VkSwapchainKHR> vkSwapChain,
                 std::array<VkFormat, colorOutputCount> surfaceFormats,
                 std::enable_if_t<Check, void*> = nullptr);

    template<bool Check = hasDepth>
    RenderTarget(vulkan::Device& device,
                 VkRenderPass vkRenderPass,
                 uint32_t swapChainLength,
                 uint32_t width,
                 uint32_t height,
                 VkFormat depthFormat,
                 std::array<VkFormat, colorOutputCount> surfaceFormats,
                 std::enable_if_t<Check, void*> = nullptr);

    template<bool Check = !hasDepth && (colorOutputCount > 0)>
    RenderTarget(vulkan::Device& device,
                 VkRenderPass vkRenderPass,
                 uint32_t swapChainLength,
                 uint32_t width,
                 uint32_t height,
                 std::array<VkFormat, colorOutputCount> surfaceFormats,
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

template<bool hasDepth, size_t colorOutputCount>
template<bool Check>
RenderTarget<hasDepth, colorOutputCount>::RenderTarget(vulkan::Device& device,
                                                       VkRenderPass vkRenderPass,
                                                       Surface& surface,
                                                       vulkan::Handle<VkSwapchainKHR>& vkSwapChain,
                                                       VkFormat depthFormat,
                                                       std::array<VkFormat, colorOutputCount> surfaceFormats,
                                                       std::enable_if_t<Check, void*>)
  : extent_{}
  , colorAttachmentCount_{ colorOutputCount }
  , swapChainLength_{ 0 }
  , currentChainIndex_{ 0 }
  , surface_{ std::move(surface) }
  , vkSwapChain_{ std::move(vkSwapChain) }
  , frameBuffers_{}
{
    extent_.width = surface_->width();
    extent_.height = surface_->height();

    uint32_t imageCount = 0;
    vkGetSwapchainImagesKHR(device.handle(), static_cast<VkSwapchainKHR>(vkSwapChain_), &imageCount, nullptr);

    swapChainLength_ = imageCount;

    std::vector<VkImage> vkImages(swapChainLength_, VK_NULL_HANDLE);

    vkGetSwapchainImagesKHR(device.handle(), static_cast<VkSwapchainKHR>(vkSwapChain_), &imageCount, vkImages.data());

    frameBuffers_.reserve(swapChainLength_);

    auto depthImagePtr = std::make_shared<vulkan::Image>(device,
                                                         extent_.width,
                                                         extent_.height,
                                                         1,
                                                         1,
                                                         1,
                                                         depthFormat,
                                                         VK_IMAGE_TILING_OPTIMAL,
                                                         VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

    for (auto vkImage : vkImages) {
        frameBuffers_.emplace_back(
          device,
          vkRenderPass,
          extent_.width,
          extent_.height,
          std::array<vulkan::ImageView, 2>{
            vulkan::ImageView{ device, depthImagePtr },
            vulkan::ImageView{
              device, std::make_shared<vulkan::Image>(vkImage, extent_.width, extent_.height, surfaceFormats[0]) } });
    }
}

template<bool hasDepth, size_t colorOutputCount>
template<bool Check>
RenderTarget<hasDepth, colorOutputCount>::RenderTarget(vulkan::Device& device,
                                                       VkRenderPass vkRenderPass,
                                                       Surface& surface,
                                                       vulkan::Handle<VkSwapchainKHR> vkSwapChain,
                                                       std::array<VkFormat, colorOutputCount> surfaceFormats,
                                                       std::enable_if_t<Check, void*>)
  : extent_{}
  , colorAttachmentCount_{ colorOutputCount }
  , swapChainLength_{ 0 }
  , currentChainIndex_{ 0 }
  , surface_{ std::move(surface) }
  , vkSwapChain_{ std::move(vkSwapChain) }
  , frameBuffers_{}
{
    extent_.width = surface_->width();
    extent_.height = surface_->height();

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
            device, std::make_shared<vulkan::Image>(vkImage, extent_.width, extent_.height, surfaceFormats[0]) } });
    }
}

template<bool hasDepth, size_t colorOutputCount>
template<bool Check>
RenderTarget<hasDepth, colorOutputCount>::RenderTarget(vulkan::Device& device,
                                                       VkRenderPass vkRenderPass,
                                                       uint32_t swapChainLength,
                                                       uint32_t width,
                                                       uint32_t height,
                                                       VkFormat depthFormat,
                                                       std::array<VkFormat, colorOutputCount> surfaceFormats,
                                                       std::enable_if_t<Check, void*>)
  : extent_{}
  , colorAttachmentCount_{ colorOutputCount }
  , swapChainLength_{ swapChainLength }
  , currentChainIndex_{ 0 }
  , surface_{}
  , vkSwapChain_{ device.handle(), vkDestroySwapchainKHR }
  , frameBuffers_{}
{
    // TODO::
    (void)vkRenderPass;
    (void)width;
    (void)height;
    (void)depthFormat;
    (void)surfaceFormats;

    throw std::runtime_error("not implemented yet");
}

template<bool hasDepth, size_t colorOutputCount>
template<bool Check>
RenderTarget<hasDepth, colorOutputCount>::RenderTarget(vulkan::Device& device,
                                                       VkRenderPass vkRenderPass,
                                                       uint32_t swapChainLength,
                                                       uint32_t width,
                                                       uint32_t height,
                                                       std::array<VkFormat, colorOutputCount> surfaceFormats,
                                                       std::enable_if_t<Check, void*>)
  : extent_{}
  , colorAttachmentCount_{ colorOutputCount }
  , swapChainLength_{ swapChainLength }
  , currentChainIndex_{ 0 }
  , surface_{}
  , vkSwapChain_{ device.handle(), vkDestroySwapchainKHR }
  , frameBuffers_{}
{
    // TODO::
    (void)vkRenderPass;
    (void)width;
    (void)height;
    (void)surfaceFormats;

    throw std::runtime_error("not implemented yet");
}

template<bool hasDepth, size_t colorOutputCount>
auto RenderTarget<hasDepth, colorOutputCount>::getColorAttachment(uint8_t attachmentIndex) const -> vulkan::ImageView const&
{
    if constexpr (hasDepth)
        return frameBuffers_[currentChainIndex_].getAttachment(attachmentIndex + 1);
    else
        return frameBuffers_[currentChainIndex_].getAttachment(attachmentIndex);
}
}

#endif // CYCLONITE_RENDERTARGET_H
