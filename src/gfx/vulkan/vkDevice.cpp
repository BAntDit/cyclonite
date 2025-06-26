//
// Created by anton on 6/26/25.
//

#include "vkDevice.h"

namespace cyclonite::gfx::vulkan {
Device::Device(ResourceManager* resourceManager,
               ResourceId resourceId,
               VkInstance vkInstance,
               VkPhysicalDevice vkPhysicalDevice,
               VkPhysicalDeviceProperties const& physicalDeviceProperties,
               std::vector<const char*> const& requiredExtensions)
  : ResourceBase{ resourceManager, resourceId, true }
  , vkInstance_{ vkInstance }
  , vkPhysicalDevice_{ vkPhysicalDevice }
  , name_{}
  , vendor_{}
  , graphicsQueueIndex_{}
  , computeQueueIndex_{}
  , deviceHostTransferQueueIndex_{}
  , vkDevice_{}
  , queueFamilyIndices_{}
  , vkQueues_{}
{
}
}
