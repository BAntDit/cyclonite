//
// Created by anton on 8/9/25.
//

#include "vkSurfaceWin32.h"

#if defined(GFX_DRIVER_VULKAN) && defined(VK_USE_PLATFORM_WIN32_KHR)

namespace cyclonite::gfx::vulkan {
SurfaceWin32::SurfaceWin32(VkInstance vkInstance, HINSTANCE hinstance, HWND hwnd)
  : SurfaceKHR{ vkInstance }
{
    auto vkWin32SurfaceCreateInfoKHR = VkWin32SurfaceCreateInfoKHR{};

    vkWin32SurfaceCreateInfoKHR.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    vkWin32SurfaceCreateInfoKHR.hinstance = hinstance;
    vkWin32SurfaceCreateInfoKHR.hwnd = hwnd;

    if (auto vkResult = vkCreateWin32SurfaceKHR(vkInstance, &vkWin32SurfaceCreateInfoKHR, nullptr, &vkSurfaceKHR_);
        vkResult != VK_SUCCESS) {
        throw Exception{ vkResult, "vkCreateWin32SurfaceKHR" };
    }
}
}

#endif
