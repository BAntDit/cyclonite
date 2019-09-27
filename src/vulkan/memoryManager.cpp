//
// Created by bantdit on 9/27/19.
//

#include "memoryManager.h"

namespace cyclonite::vulkan {
MemoryManager::MemoryManager(cyclonite::multithreading::TaskManager const& taskManager,
                             cyclonite::vulkan::Device const& device)
  : taskManager_{ &taskManager }
{
    VkPhysicalDeviceMemoryProperties vkPhysicalDeviceMemoryProperties = {};
    vkGetPhysicalDeviceMemoryProperties(device.physicalDevice(), &vkPhysicalDeviceMemoryProperties);
}
}
