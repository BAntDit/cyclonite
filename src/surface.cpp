//
// Created by bantdit on 11/13/19.
//

#include "surface.h"

namespace cyclonite {
Surface::Surface(vulkan::Device const& device, Options::WindowProperties const& windowProperties)
  : capabilities_{}
  , extent_{}
  , window_{ windowProperties.title,
             windowProperties.left,
             windowProperties.top,
             windowProperties.width,
             windowProperties.height,
             static_cast<uint32_t>(windowProperties.fullscreen
                                     ? SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN | SDL_WINDOW_BORDERLESS
                                     : SDL_WINDOW_SHOWN) }
  , platformSurface_{ _createSurface(device.vulkanInstance(),
                                     window_,
                                     vulkan::platform_surface_argument_type_list_t{}) }
  , vkSwapchain_{ device.handle(), vkDestroySwapchainKHR }
  , imageViews_{}
  , imageAvailableSemaphores_{}
  , vkFormat_{ VK_FORMAT_UNDEFINED }
  , vkColorSpaceKHR_{ VK_COLOR_SPACE_MAX_ENUM_KHR }
{
    VkBool32 presentationSupport = VK_FALSE;

    if (auto result = vkGetPhysicalDeviceSurfaceSupportKHR(
          device.physicalDevice(), device.graphicsQueueFamilyIndex(), platformSurface_.handle(), &presentationSupport);
        result != VK_SUCCESS) {
        if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
            throw std::runtime_error("device has no enough memory to test surface support");
        }

        if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
            throw std::runtime_error("device has no enough GPU memory to test surface support");
        }

        if (result == VK_ERROR_SURFACE_LOST_KHR) {
            throw std::runtime_error("surface lost");
        }

        std::terminate();
    }

    if (presentationSupport == VK_FALSE) {
        throw std::runtime_error("device graphics queue does not support surface");
    }

    VkSurfaceCapabilitiesKHR vkSurfaceCapabilitiesKHR = {};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
      device.physicalDevice(), platformSurface_.handle(), &vkSurfaceCapabilitiesKHR);

    if (vkSurfaceCapabilitiesKHR.currentExtent.width != std::numeric_limits<uint32_t>::max() &&
        vkSurfaceCapabilitiesKHR.currentExtent.height != std::numeric_limits<uint32_t>::max()) {
        extent_.width = vkSurfaceCapabilitiesKHR.currentExtent.width;
        extent_.height = vkSurfaceCapabilitiesKHR.currentExtent.height;
    } else {
        extent_.width = std::max(vkSurfaceCapabilitiesKHR.minImageExtent.width,
                                 std::min(vkSurfaceCapabilitiesKHR.maxImageExtent.width,
                                          static_cast<uint32_t>(windowProperties.width - windowProperties.left)));
        extent_.height = std::max(vkSurfaceCapabilitiesKHR.minImageExtent.height,
                                  std::min(vkSurfaceCapabilitiesKHR.maxImageExtent.height,
                                           static_cast<uint32_t>(windowProperties.height - windowProperties.top)));
    }

    capabilities_.maxImageCount = vkSurfaceCapabilitiesKHR.maxImageCount;
    capabilities_.minImageCount = vkSurfaceCapabilitiesKHR.minImageCount;
    capabilities_.supportedCompositeAlpha = vkSurfaceCapabilitiesKHR.supportedCompositeAlpha;
    capabilities_.supportedUsageFlags = vkSurfaceCapabilitiesKHR.supportedUsageFlags;
    capabilities_.currentTransform = vkSurfaceCapabilitiesKHR.currentTransform;
    capabilities_.supportedTransforms = vkSurfaceCapabilitiesKHR.supportedTransforms;

    uint32_t imageCount = 0;
    vkGetSwapchainImagesKHR(device.handle(), static_cast<VkSwapchainKHR>(vkSwapchain_), &imageCount, nullptr);

    std::vector<VkImage> vkImages(imageCount, VK_NULL_HANDLE);

    vkGetSwapchainImagesKHR(device.handle(), static_cast<VkSwapchainKHR>(vkSwapchain_), &imageCount, vkImages.data());

    imageViews_.reserve(imageCount);

    for (auto vkImage : vkImages) {
        imageViews_.emplace_back(
          device,
          std::make_shared<vulkan::Image>(vkImage, actualExtent.width, actualExtent.height, surfaceFormat.format));
    }

    vkFormat_ = surfaceFormat.format;
    vkColorSpaceKHR_ = surfaceFormat.colorSpace;

    {
        imageAvailableSemaphores_.resize(imageCount,
                                         vulkan::Handle<VkSemaphore>{ device.handle(), vkDestroySemaphore });

        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        for (auto& vkSemaphore : imageAvailableSemaphores_) {
            if (auto result = vkCreateSemaphore(device.handle(), &semaphoreInfo, nullptr, &vkSemaphore);
                result != VK_SUCCESS) {
                throw std::runtime_error("failed to create acquire image synchronization semaphores");
            }
        }
    }
}
}
