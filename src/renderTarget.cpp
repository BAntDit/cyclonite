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
  , frontBufferIndex_{ 0 }
  , backBufferIndex_{ 0 }
  , surface_{ std::move(surface) }
  , vkSwapChain_{ std::move(vkSwapChain) }
  , frameBuffers_{}
  , imageAvailableSemaphores_{}
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

    imageAvailableSemaphores_.reserve(swapChainLength_);

    VkSemaphoreCreateInfo semaphoreCreateInfo = {};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

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
        std::vector<vulkan::ImageView> attachments = {};

        attachments.reserve(2);

        attachments.emplace_back(device, depthImagePtr, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_DEPTH_BIT);
        attachments.emplace_back(
          device, std::make_shared<vulkan::Image>(vkImage, extent_.width, extent_.height, surfaceFormat));

        frameBuffers_.emplace_back(device, vkRenderPass, extent_.width, extent_.height, std::move(attachments));

        if (auto result =
              vkCreateSemaphore(device.handle(),
                                &semaphoreCreateInfo,
                                nullptr,
                                &imageAvailableSemaphores_.emplace_back(device.handle(), vkDestroySemaphore));
            result != VK_SUCCESS) {
            throw std::runtime_error("could not create image available semaphore.");
        }
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
  , frontBufferIndex_{ 0 }
  , backBufferIndex_{ 0 }
  , surface_{ std::move(surface) }
  , vkSwapChain_{ std::move(vkSwapChain) }
  , frameBuffers_{}
  , imageAvailableSemaphores_{}
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
    imageAvailableSemaphores_.reserve(swapChainLength_);

    VkSemaphoreCreateInfo semaphoreCreateInfo = {};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    for (auto vkImage : vkImages) {
        std::vector<vulkan::ImageView> attachments = {};

        attachments.emplace_back(
          device, std::make_shared<vulkan::Image>(vkImage, extent_.width, extent_.height, surfaceFormat));

        frameBuffers_.emplace_back(device, vkRenderPass, extent_.width, extent_.height, std::move(attachments));

        if (auto result =
              vkCreateSemaphore(device.handle(),
                                &semaphoreCreateInfo,
                                nullptr,
                                &imageAvailableSemaphores_.emplace_back(device.handle(), vkDestroySemaphore));
            result != VK_SUCCESS) {
            throw std::runtime_error("could not create image available semaphore.");
        }
    }
}

auto RenderTarget::hasAttachment(RenderTargetOutputSemantic semantic) const -> bool
{
    return outputSemantics_.count(semantic) > 0;
}

auto RenderTarget::hasAttachment(uint8_t attachmentIndex) const -> bool
{
    return hasDepth_ ? static_cast<size_t>(attachmentIndex + 1) < outputSemantics_.size()
                     : static_cast<size_t>(attachmentIndex) < outputSemantics_.size();
}

auto RenderTarget::getColorAttachment(uint8_t attachmentIndex) const -> vulkan::ImageView const&
{
    assert(hasAttachment(attachmentIndex));
    return frameBuffers_[frontBufferIndex_].getAttachment(hasDepth_ ? attachmentIndex + 1 : attachmentIndex);
}

auto RenderTarget::getColorAttachment(RenderTargetOutputSemantic semantic) const -> vulkan::ImageView const&
{
    assert(hasAttachment(semantic));
    return frameBuffers_[frontBufferIndex_].getAttachment(outputSemantics_.find(semantic)->second);
}

auto RenderTarget::getDepthAttachment() const -> vulkan::ImageView const&
{
    assert(hasDepth());
    return frameBuffers_[frontBufferIndex_].getAttachment(0);
}

auto RenderTarget::frontBufferAvailableSemaphore() const -> VkSemaphore
{
    return static_cast<VkSemaphore>(imageAvailableSemaphores_[frontBufferIndex_]);
}

auto RenderTarget::acquireBackBufferIndex(vulkan::Device const& device) -> size_t
{
    uint32_t imageIndex = 0;

    vkAcquireNextImageKHR(device.handle(),
                          static_cast<VkSwapchainKHR>(vkSwapChain_),
                          std::numeric_limits<uint64_t>::max(),
                          static_cast<VkSemaphore>(imageAvailableSemaphores_[frontBufferIndex_]),
                          VK_NULL_HANDLE,
                          &imageIndex);

    return backBufferIndex_ = imageIndex;
}

void RenderTarget::swapBuffers(vulkan::Device const& device, VkSemaphore passFinishedSemaphore)
{
    auto imageIndex = static_cast<uint32_t>(backBufferIndex_);

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &passFinishedSemaphore;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &vkSwapChain_;
    presentInfo.pImageIndices = &imageIndex;

    vkQueuePresentKHR(device.graphicsQueue(), &presentInfo);

    frontBufferIndex_ = (frontBufferIndex_ + 1) % swapChainLength_;
}
}
