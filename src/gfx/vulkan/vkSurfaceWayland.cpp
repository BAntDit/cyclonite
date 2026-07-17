//
// Created by anton on 8/9/25.
//

#include "vkSurfaceWayland.h"

#if defined(GFX_DRIVER_VULKAN) && defined(VK_USE_PLATFORM_WAYLAND_KHR)
namespace cyclonite::gfx::vulkan {
SurfaceWayland::SurfaceWayland(VkInstance vkInstance, wl_display* display, wl_surface* surface)
  : SurfaceKHR{ vkInstance }
{
    auto vkWaylandSurfaceCreateInfoKHR = VkWaylandSurfaceCreateInfoKHR{};
    vkWaylandSurfaceCreateInfoKHR.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
    vkWaylandSurfaceCreateInfoKHR.display = display;
    vkWaylandSurfaceCreateInfoKHR.surface = surface;

    if (auto vkResult = vkCreateWaylandSurfaceKHR(vkInstance, &vkWaylandSurfaceCreateInfoKHR, nullptr, &vkSurfaceKHR_);
        vkResult != VK_SUCCESS) {
        throw Exception{ vkResult, "vkCreateWaylandSurfaceKHR" };
    }
}
}
#endif
