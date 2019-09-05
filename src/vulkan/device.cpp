//
// Created by bantdit on 9/5/19.
//

#include "device.h"

namespace cyclonite::vulkan {
Device::Device(VkPhysicalDevice const& vkPhysicalDevice)
  : name_{ "" }
  , vkPhysicalDevice_{ vkPhysicalDevice }
  , vkDeviceProperties_{}
  , vkPhysicalDeviceMemoryProperties_{}
{
    vkGetPhysicalDeviceProperties(vkPhysicalDevice_, &vkDeviceProperties_);
    vkGetPhysicalDeviceMemoryProperties(vkPhysicalDevice_, &vkPhysicalDeviceMemoryProperties_);

    VkPhysicalDeviceFeatures features = {};

    // turn off unused features (for now)
    features.robustBufferAccess = VK_FALSE;
    features.shaderFloat64 = VK_FALSE;
    features.shaderInt64 = VK_FALSE;
    features.inheritedQueries = VK_FALSE;
}
}
