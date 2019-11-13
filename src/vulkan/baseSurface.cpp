//
// Created by bantdit on 11/13/19.
//

#include "baseSurface.h"

namespace cyclonite::vulkan {
BaseSurface::BaseSurface(VkInstance vkInstance)
  : vkSurfaceKHR_{ vkInstance, vkDestroySurfaceKHR }
{}
}
