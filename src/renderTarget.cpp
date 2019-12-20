//
// Created by bantdit on 12/6/19.
//

#include "renderTarget.h"

namespace cyclonite {
RenderTarget::RenderTarget(vulkan::Device const& device,
                           VkRenderPass vkRenderPass,
                           Surface& surface,
                           vulkan::Handle<VkSwapchainKHR> swapChain,
                           VkFormat depthFormat,
                           VkFormat surfaceFormat)
  : extent_{}
  , colorAttachmentCount_{ 1 }
  , swapChainLength_{ 0 }
  , currentChainIndex_{ 0 }
  , surface_{ std::move(surface) }
  , vkSwapChain_{ std::move(swapChain) }
  , frameBuffers_{}
{
    uint32_t imageCount = 0;
    vkGetSwapchainImagesKHR(device.handle(), static_cast<VkSwapchainKHR>(vkSwapChain_), &imageCount, nullptr);

    swapChainLength_ = imageCount;

    std::vector<VkImage> vkImages(swapChainLength_, VK_NULL_HANDLE);

    vkGetSwapchainImagesKHR(device.handle(), static_cast<VkSwapchainKHR>(vkSwapChain_), &imageCount, vkImages.data());

    frameBuffers_.reserve(swapChainLength_);

    if (depthFormat != VK_FORMAT_UNDEFINED) {
        // TODO:: ...
    } else {
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

auto RenderTarget::getColorAttachment(uint8_t attachmentIndex) const -> vulkan::ImageView const&
{
    assert(attachmentIndex < colorAttachmentCount_);

    assert(imageViews_.size() < currentChainIndex_ * colorAttachmentCount_ + attachmentIndex);

    return imageViews_[currentChainIndex_ * colorAttachmentCount_ + attachmentIndex];
}
}
