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
                                         VkClearDepthStencilValue clearDepthStencilValue,
                                         VkFormat surfaceFormat,
                                         VkClearColorValue clearSurfaceValue,
                                         RenderTargetOutputSemantic outputSemantic)
  : BaseRenderTarget(surface.width(), surface.height(), clearDepthStencilValue, std::array{ clearSurfaceValue })
  , surface_{ std::move(surface) }
  , vkSwapChain_{ std::move(vkSwapChain) }
  , imageAvailableSemaphores_{}
{
    outputSemantics_[outputSemantic] = 0;

    uint32_t imageCount = 0;
    vkGetSwapchainImagesKHR(device.handle(), static_cast<VkSwapchainKHR>(vkSwapChain_), &imageCount, nullptr);

    swapChainLength_ = imageCount;

    std::vector<VkImage> vkImages(swapChainLength_, VK_NULL_HANDLE);

    vkGetSwapchainImagesKHR(device.handle(), static_cast<VkSwapchainKHR>(vkSwapChain_), &imageCount, vkImages.data());

    frameBuffers_.reserve(swapChainLength_);

    imageAvailableSemaphores_.reserve(swapChainLength_);

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
                                         VkClearColorValue clearSurfaceValue,
                                         RenderTargetOutputSemantic outputSemantic)
  : BaseRenderTarget(surface.width(), surface.height(), std::array{ clearSurfaceValue })
  , surface_{ std::move(surface) }
  , vkSwapChain_{ std::move(vkSwapChain) }
  , imageAvailableSemaphores_{}
{
    outputSemantics_[outputSemantic] = 0;

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
    }
}

auto SurfaceRenderTarget::acquireBackBufferIndex(vulkan::Device const& device) -> size_t
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

void SurfaceRenderTarget::swapBuffers(vulkan::Device const& device, VkSemaphore passFinishedSemaphore)
{
    auto imageIndex = static_cast<uint32_t>(backBufferIndex_);

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &passFinishedSemaphore;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &std::as_const(vkSwapChain_);
    presentInfo.pImageIndices = &imageIndex;

    vkQueuePresentKHR(device.graphicsQueue(), &presentInfo);

    frontBufferIndex_ = (frontBufferIndex_ + 1) % swapChainLength_;
}
}
