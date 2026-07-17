//
// Created by anton on 8/9/25.
//

#include "vkSurfaceAndroid.h"

#if defined(GFX_DRIVER_VULKAN) && defined(VK_USE_PLATFORM_ANDROID_KHR)

namespace cyclonite::gfx::vulkan {
SurfaceAndroid::SurfaceAndroid(VkInstance vkInstance, ANativeWindow* window)
  : SurfaceKHR{ vkInstance }
{
    auto vkAndroidSurfaceCreateInfoKHR = VkAndroidSurfaceCreateInfoKHR{};
    vkAndroidSurfaceCreateInfoKHR.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
    vkAndroidSurfaceCreateInfoKHR.window = window;

    if (auto vkResult = vkCreateAndroidSurfaceKHR(vkInstance, &vkAndroidSurfaceCreateInfoKHR, nullptr, &vkSurfaceKHR_);
        vkResult != VK_SUCCESS) {
        throw Exception{ vkResult, "vkCreateAndroidSurfaceKHR" };
    }
}
}

#endif
