//
// Created by bantdit on 12/6/19.
//

#include "renderTarget.h"

namespace cyclonite {
RenderTarget::RenderTarget(vulkan::Device& device,
                           VkRenderPass vkRenderPass,
                           Surface& surface,
                           vulkan::Handle<VkSwapchainKHR>& vkSwapChain,
                           VkFormat depthFormat,
                           VkFormat surfaceFormat,
                           RenderTargetOutputSemantic outputSemantic)
  : extent_{}
  , colorAttachmentCount_{ 1 }
  , swapChainLength_{ 0 }
  , currentChainIndex_{ 0 }
  , surface_{ std::move(surface) }
  , vkSwapChain_{ std::move(vkSwapChain) }
  , frameBuffers_{}
  , outputSemantics_{ { outputSemantic, 1 } }
  , hasDepth_{ true }
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
          std::vector{
            vulkan::ImageView{ device, depthImagePtr },
            vulkan::ImageView{
              device, std::make_shared<vulkan::Image>(vkImage, extent_.width, extent_.height, surfaceFormat) } });
    }
}

RenderTarget::RenderTarget(vulkan::Device& device,
                           VkRenderPass vkRenderPass,
                           Surface& surface,
                           vulkan::Handle<VkSwapchainKHR> vkSwapChain,
                           VkFormat surfaceFormat,
                           RenderTargetOutputSemantic outputSemantic)
  : extent_{}
  , colorAttachmentCount_{ 1 }
  , swapChainLength_{ 0 }
  , currentChainIndex_{ 0 }
  , surface_{ std::move(surface) }
  , vkSwapChain_{ std::move(vkSwapChain) }
  , frameBuffers_{}
  , outputSemantics_{ { outputSemantic, 0 } }
  , hasDepth_{ false }
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
          std::vector{ vulkan::ImageView{
            device, std::make_shared<vulkan::Image>(vkImage, extent_.width, extent_.height, surfaceFormat) } });
    }
}

auto RenderTarget::hasAttachment(RenderTargetOutputSemantic semantic) const -> bool
{
    return outputSemantics_.count(semantic) > 0;
}

auto RenderTarget::hasAttachment(uint8_t attachmentIndex) const -> bool
{
    return hasDepth_ ? (attachmentIndex + 1) < outputSemantics_.size() : attachmentIndex < outputSemantics_.size();
}

auto RenderTarget::getColorAttachment(uint8_t attachmentIndex) const -> vulkan::ImageView const&
{
    assert(hasAttachment(attachmentIndex));
    return frameBuffers_[currentChainIndex_].getAttachment(hasDepth_ ? attachmentIndex + 1 : attachmentIndex);
}

auto RenderTarget::getColorAttachment(RenderTargetOutputSemantic semantic) const -> vulkan::ImageView const&
{
    assert(hasAttachment(semantic));
    return frameBuffers_[currentChainIndex_].getAttachment(outputSemantics_.find(semantic)->second);
}

auto RenderTarget::getDepthAttachment() const -> vulkan::ImageView const&
{
    assert(hasDepth());
    return frameBuffers_[currentChainIndex_].getAttachment(0);
}
}
