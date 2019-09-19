//
// Created by bantdit on 9/19/19.
//

#include "androidSurface.h"

#if defined(VK_USE_PLATFORM_WAYLAND_KHR)

namespace cyclonite::vulkan {
AndroidSurface::AndroidSurface(VkInstance vkInstance, ANativeWindow* window)
  : vkSurfaceKHR_{ vkInstance, vkDestroySurfaceKHR }
{
    VkAndroidSurfaceCreateInfoKHR vkAndroidSurfaceCreateInfoKHR = {};

    vkAndroidSurfaceCreateInfoKHR.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
    vkAndroidSurfaceCreateInfoKHR.window = window;

    auto result = vkCreateAndroidSurfaceKHR(vkInstance, &vkAndroidSurfaceCreateInfoKHR, nullptr, &vkSurfaceKHR_);

    if (result != VK_SUCCESS) {
        if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
            throw std::runtime_error("there is no enough memory on device to create andriod surface");
        }

        if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
            throw std::runtime_error("there is no enough memory on host to create andriod surface");
        }
    }
}
}

#endif
