//
// Created by bantdit on 11/13/19.
//

#include "surface.h"

namespace cyclonite {
Surface::Surface(VkInstance vkInstance,
                 VkPhysicalDevice physicalDevice,
                 const cyclonite::Options::WindowProperties& windowProperties)
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
    VkSurfaceCapabilitiesKHR vkSurfaceCapabilitiesKHR = {};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, platformSurface_.handle(), &vkSurfaceCapabilitiesKHR);

    capabilities_.minSwapChainImageCount = vkSurfaceCapabilitiesKHR.minImageCount;
    capabilities_.maxSwapChainImageCount = vkSurfaceCapabilitiesKHR.maxImageCount;
}
}
