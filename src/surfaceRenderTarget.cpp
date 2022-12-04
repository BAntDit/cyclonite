//
// Created by bantdit on 1/7/20.
//

#include "surfaceRenderTarget.h"

namespace cyclonite {
SurfaceRenderTarget::SurfaceRenderTarget(vulkan::Device& device,
                                         VkRenderPass vkRenderPass,
                                         Surface& surface,
                                         vulkan::Handle<VkSwapchainKHR>& vkSwapChain,
                                         VkFormat depthStencilFormat,
                                         VkFormat surfaceFormat,
                                         RenderTargetOutputSemantic outputSemantic)
  : BaseRenderTarget(surface.width(), surface.height(), 1, true)
  , surface_{ std::move(surface) }
  , vkSwapChain_{ std::move(vkSwapChain) }
  , imageAvailableSemaphores_{}
  , currentImageIndex_{ std::numeric_limits<uint32_t>::max() }
{
    outputSemantics_[outputSemantic] = 0;

    uint32_t bufferCount = 0;
    vkGetSwapchainImagesKHR(device.handle(), static_cast<VkSwapchainKHR>(vkSwapChain_), &bufferCount, nullptr);

    std::vector<VkImage> vkImages(bufferCount, VK_NULL_HANDLE);

    vkGetSwapchainImagesKHR(device.handle(), static_cast<VkSwapchainKHR>(vkSwapChain_), &bufferCount, vkImages.data());

    frameBuffers_.reserve(bufferCount);

    imageAvailableSemaphores_.reserve(bufferCount);

    auto depthImagePtr = std::make_shared<vulkan::Image>(device,
                                                         width(),
                                                         height(),
                                                         1,
                                                         1,
                                                         1,
                                                         depthStencilFormat,
                                                         VK_IMAGE_TILING_OPTIMAL,
                                                         VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

    VkSemaphoreCreateInfo semaphoreCreateInfo = {};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    for (auto vkImage : vkImages) {
        frameBuffers_.emplace_back(
          device,
          vkRenderPass,
          width(),
          height(),
          vulkan::ImageView{ device, depthImagePtr, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_DEPTH_BIT },
          std::array{
            vulkan::ImageView{ device, std::make_shared<vulkan::Image>(vkImage, width(), height(), surfaceFormat) } });

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

SurfaceRenderTarget::SurfaceRenderTarget(vulkan::Device& device,
                                         VkRenderPass vkRenderPass,
                                         Surface& surface,
                                         vulkan::Handle<VkSwapchainKHR>& vkSwapChain,
                                         VkFormat surfaceFormat,
                                         RenderTargetOutputSemantic outputSemantic)
  : BaseRenderTarget(surface.width(), surface.height())
  , surface_{ std::move(surface) }
  , vkSwapChain_{ std::move(vkSwapChain) }
  , imageAvailableSemaphores_{}
  , imageReadyToBePresentedSemaphore_{}
  , currentImageIndex_{ std::numeric_limits<uint32_t>::max() }
{
    outputSemantics_[outputSemantic] = 0;

    uint32_t bufferCount = 0;
    vkGetSwapchainImagesKHR(device.handle(), static_cast<VkSwapchainKHR>(vkSwapChain_), &bufferCount, nullptr);

    std::vector<VkImage> vkImages(bufferCount, VK_NULL_HANDLE);

    vkGetSwapchainImagesKHR(device.handle(), static_cast<VkSwapchainKHR>(vkSwapChain_), &bufferCount, vkImages.data());

    frameBuffers_.reserve(bufferCount);

    imageAvailableSemaphores_.reserve(bufferCount);
    imageReadyToBePresentedSemaphore_.reserve(bufferCount);

    VkSemaphoreCreateInfo semaphoreCreateInfo = {};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    for (auto vkImage : vkImages) {
        frameBuffers_.emplace_back(
          device,
          vkRenderPass,
          width(),
          height(),
          std::array{
            vulkan::ImageView{ device, std::make_shared<vulkan::Image>(vkImage, width(), height(), surfaceFormat) } });

        if (auto result =
              vkCreateSemaphore(device.handle(),
                                &semaphoreCreateInfo,
                                nullptr,
                                &imageAvailableSemaphores_.emplace_back(device.handle(), vkDestroySemaphore));
            result != VK_SUCCESS) {
            throw std::runtime_error("could not create image available semaphore.");
        }

        if (auto result =
              vkCreateSemaphore(device.handle(),
                                &semaphoreCreateInfo,
                                nullptr,
                                &imageReadyToBePresentedSemaphore_.emplace_back(device.handle(), vkDestroySemaphore));
            result != VK_SUCCESS) {
            throw std::runtime_error("could not create image ready semaphore.");
        }
    }
}

auto SurfaceRenderTarget::acquireBackBufferIndex(vulkan::Device const& device, uint32_t frameIndex)
  -> std::pair<uint32_t, VkSemaphore>
{
    auto wait = static_cast<VkSemaphore>(imageAvailableSemaphores_[frameIndex]);

    vkAcquireNextImageKHR(device.handle(),
                          static_cast<VkSwapchainKHR>(vkSwapChain_),
                          std::numeric_limits<uint64_t>::max(),
                          wait,
                          VK_NULL_HANDLE,
                          &currentImageIndex_);

    return std::make_pair(currentImageIndex_, wait);
}

void SurfaceRenderTarget::swapBuffers(vulkan::Device const& device, vulkan::Handle<VkSemaphore> const& signal)
{
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &imageReadyToBePresentedSemaphore_[currentImageIndex_];
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &std::as_const(vkSwapChain_);

    // image at fixedPartIndex currentFrameImageIndex gets available after signal
    presentInfo.pImageIndices = &currentImageIndex_;

    vkQueuePresentKHR(device.graphicsQueue(), &presentInfo);
}
auto SurfaceRenderTarget::signal() const -> VkSemaphore
{
    return static_cast<VkSemaphore>(imageReadyToBePresentedSemaphore_[currentImageIndex_]);
}
}
