//
// Created by anton on 8/9/25.
//

#include "vkSurfaceXlib.h"
#include "vkException.h"

#if defined(GFX_DRIVER_VULKAN) && defined(VK_USE_PLATFORM_XLIB_KHR)
namespace cyclonite::gfx::vulkan {
SurfaceXlib::SurfaceXlib(VkInstance vkInstance, Display* display, Window window)
  : SurfaceKHR{ vkInstance }
{
    auto vkXlibSurfaceCreateInfoKHR = VkXlibSurfaceCreateInfoKHR{};

    vkXlibSurfaceCreateInfoKHR.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
    vkXlibSurfaceCreateInfoKHR.dpy = display;
    vkXlibSurfaceCreateInfoKHR.window = window;

    if (auto vkResult = vkCreateXlibSurfaceKHR(vkInstance, &vkXlibSurfaceCreateInfoKHR, nullptr, &vkSurfaceKHR_);
        vkResult != VK_SUCCESS) {
        throw Exception{ vkResult, "vkCreateXlibSurfaceKHR" };
    }
}
}

#endif
