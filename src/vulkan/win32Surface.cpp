//
// Created by bantdit on 9/19/19.
//

#include "win32Surface.h"

#if defined(VK_USE_PLATFORM_WIN32_KHR)

namespace cyclonite::vulkan {
Win32Surface::Win32Surface(VkInstance vkInstance, HINSTANCE hinstance, HWND hwnd)
  : vkSurfaceKHR_{ vkInstance, vkDestroySurfaceKHR }
{
    VkWin32SurfaceCreateInfoKHR vkWin32SurfaceCreateInfoKHR = {};

    vkWin32SurfaceCreateInfoKHR.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    vkWin32SurfaceCreateInfoKHR.hinstance = hinstance;
    vkWin32SurfaceCreateInfoKHR.hwnd = hwnd;

    auto result = vkCreateWin32SurfaceKHR(vkInstance, &vkWin32SurfaceCreateInfoKHR, nullptr, &vkSurfaceKHR_);

    if (result != VK_SUCCESS) {
        if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
            throw std::runtime_error("there is no enough memory on device to create win32 surface");
        }

        if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
            throw std::runtime_error("there is no enough memory on host to create win32 surface");
        }
    }
}
}

#endif
