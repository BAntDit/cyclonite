//
// Created by bantdit on 9/19/19.
//

#include "wlSurface.h"

#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
namespace cyclonite::vulkan {
WlSurface::WlSurface(VkInstance vkInstance, wl_display* display, wl_surface* surface)
  : BaseSurface{ vkInstance }
{
    VkWaylandSurfaceCreateInfoKHR vkWaylandSurfaceCreateInfoKHR = {};

    vkWaylandSurfaceCreateInfoKHR.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
    vkWaylandSurfaceCreateInfoKHR.display = display;
    vkWaylandSurfaceCreateInfoKHR.surface = surface;

    auto result = vkCreateWaylandSurfaceKHR(vkInstance, &vkWaylandSurfaceCreateInfoKHR, nullptr, &vkSurfaceKHR_);

    if (result != VK_SUCCESS) {
        if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
            throw std::runtime_error("there is no enough memory on device to create wayland surface");
        }

        if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
            throw std::runtime_error("there is no enough memory on host to create wayland surface");
        }
    }
}
}

#endif
