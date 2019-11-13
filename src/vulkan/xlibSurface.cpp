//
// Created by bantdit on 9/17/19.
//

#include "xlibSurface.h"

#if defined(VK_USE_PLATFORM_XLIB_KHR)

namespace cyclonite::vulkan {
XlibSurface::XlibSurface(VkInstance vkInstance, Display* display, Window const& window)
  : BaseSurface{ vkInstance }
{
    VkXlibSurfaceCreateInfoKHR vkXlibSurfaceCreateInfoKHR = {};

    vkXlibSurfaceCreateInfoKHR.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
    vkXlibSurfaceCreateInfoKHR.dpy = display;
    vkXlibSurfaceCreateInfoKHR.window = window;

    auto result = vkCreateXlibSurfaceKHR(vkInstance, &vkXlibSurfaceCreateInfoKHR, nullptr, &vkSurfaceKHR_);

    if (result != VK_SUCCESS) {
        if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
            throw std::runtime_error("there is no enough memory on device to create xlib surface");
        }

        if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
            throw std::runtime_error("there is no enough memory on host to create xlib surface");
        }
    }
}
}

#endif
