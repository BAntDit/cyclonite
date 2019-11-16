//
// Created by bantdit on 11/13/19.
//

#include "surface.h"

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
  , capabilities_{}
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

    capabilities_.minSwapChainImageCount = vkSurfaceCapabilitiesKHR.minImageCount;
    capabilities_.maxSwapChainImageCount = vkSurfaceCapabilitiesKHR.maxImageCount;
}
}
