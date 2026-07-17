//
// Created by anton on 7/24/25.
//

#include "vkSurfaceKHR.h"

#if defined(GFX_DRIVER_VULKAN)

namespace cyclonite::gfx::vulkan {
SurfaceKHR::SurfaceKHR(VkInstance vkInstance)
  : vkSurfaceKHR_{ vkInstance, vkDestroySurfaceKHR }
{
}
}

#endif // GFX_DRIVER_VULKAN
