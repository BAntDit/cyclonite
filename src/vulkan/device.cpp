//
// Created by bantdit on 9/5/19.
//

#include "device.h"

namespace cyclonite::vulkan {
Device::Device(VkPhysicalDevice const& vkPhysicalDevice)
  : vkPhysicalDevice_{ vkPhysicalDevice }
  , vkDeviceProperties_{}
  , vkPhysicalDeviceMemoryProperties_{}
{
    vkGetPhysicalDeviceProperties(vkPhysicalDevice_, &vkDeviceProperties_);
    vkGetPhysicalDeviceMemoryProperties(vkPhysicalDevice_, &vkPhysicalDeviceMemoryProperties_);
}
}
