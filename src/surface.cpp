//
// Created by bantdit on 11/13/19.
//

#include "surface.h"
#include "internal/surfaceUtils.h"

namespace cyclonite {
Surface::Surface(VkInstance vkInstance,
                 vulkan::Device const& device,
                 cyclonite::Options::WindowProperties const& windowProperties)
  : window_{ windowProperties.title,
             windowProperties.left,
             windowProperties.top,
             windowProperties.width,
             windowProperties.height,
             static_cast<uint32_t>(windowProperties.fullscreen
                                     ? SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN | SDL_WINDOW_BORDERLESS
                                     : SDL_WINDOW_SHOWN) }
  , platformSurface_{ _createSurface(vkInstance, window_, vulkan::platform_surface_argument_type_list_t{}) }
  , vkSwapchain_{ device.handle(), vkDestroySwapchainKHR }
  , imageViews_{}
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

    auto surfaceFormat =
      internal::_chooseSurfaceFormat(device.physicalDevice(),
                                     platformSurface_.handle(),
                                     std::array<std::pair<VkFormat, VkColorSpaceKHR>, 1>{
                                       std::make_pair(VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) });

    auto presentationMode = internal::_choosePresentationMode(
      device.physicalDevice(),
      platformSurface_.handle(),
      std::array<VkPresentModeKHR, 2>{ VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_FIFO_KHR });

    VkExtent2D actualExtent = {};

    if (vkSurfaceCapabilitiesKHR.currentExtent.width != std::numeric_limits<uint32_t>::max() &&
        vkSurfaceCapabilitiesKHR.currentExtent.height != std::numeric_limits<uint32_t>::max()) {
        actualExtent.width = vkSurfaceCapabilitiesKHR.currentExtent.width;
        actualExtent.height = vkSurfaceCapabilitiesKHR.currentExtent.height;
    } else {
        actualExtent.width = std::max(vkSurfaceCapabilitiesKHR.minImageExtent.width,
                                      std::min(vkSurfaceCapabilitiesKHR.maxImageExtent.width,
                                               static_cast<uint32_t>(windowProperties.width - windowProperties.left)));
        actualExtent.height = std::max(vkSurfaceCapabilitiesKHR.minImageExtent.height,
                                       std::min(vkSurfaceCapabilitiesKHR.maxImageExtent.height,
                                                static_cast<uint32_t>(windowProperties.height - windowProperties.top)));
    }

    VkSwapchainCreateInfoKHR swapchainCreateInfoKHR = {};
    swapchainCreateInfoKHR.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfoKHR.surface = platformSurface_.handle();
    swapchainCreateInfoKHR.minImageCount =
      std::min(vkSurfaceCapabilitiesKHR.minImageCount + 1,
               vkSurfaceCapabilitiesKHR.maxImageCount > 0 ? vkSurfaceCapabilitiesKHR.maxImageCount
                                                          : std::numeric_limits<uint32_t>::max());
    swapchainCreateInfoKHR.imageFormat = surfaceFormat.format;
    swapchainCreateInfoKHR.imageColorSpace = surfaceFormat.colorSpace;
    swapchainCreateInfoKHR.imageExtent = actualExtent;
    swapchainCreateInfoKHR.imageArrayLayers = 1;
    swapchainCreateInfoKHR.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCreateInfoKHR.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainCreateInfoKHR.preTransform = vkSurfaceCapabilitiesKHR.currentTransform;
    swapchainCreateInfoKHR.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfoKHR.presentMode = presentationMode;
    swapchainCreateInfoKHR.clipped = VK_TRUE;
    swapchainCreateInfoKHR.oldSwapchain = VK_NULL_HANDLE;

    if (auto result = vkCreateSwapchainKHR(device.handle(), &swapchainCreateInfoKHR, nullptr, &vkSwapchain_);
        result != VK_SUCCESS) {
        if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
            throw std::runtime_error("not enough RAM memory to create swap chain");
        }

        if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
            throw std::runtime_error("not enough GPU memory to create swap chain");
        }

        if (result == VK_ERROR_DEVICE_LOST) {
            throw std::runtime_error("device lost on attempt to create swap chain");
        }

        if (result == VK_ERROR_SURFACE_LOST_KHR) {
            throw std::runtime_error("surface lost");
        }

        if (result == VK_ERROR_NATIVE_WINDOW_IN_USE_KHR) {
            throw std::runtime_error("could not create swap chain - native window is in use");
        }

        if (result == VK_ERROR_INITIALIZATION_FAILED) {
            throw std::runtime_error("swap chain initialization failed");
        }

        std::terminate();
    }

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
}
}
