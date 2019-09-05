//
// Created by bantdit on 9/5/19.
//

#include "device.h"

namespace cyclonite::vulkan {
static uint32_t getBestQueueFamilyIndex(VkPhysicalDevice const& physicalDevice,
                                        const VkQueueFlags& flags,
                                        unsigned int preciseCapabilitiesCount = 0)
{
    uint32_t queueFamilyIndex = std::numeric_limits<uint32_t>::max();

    uint32_t familyCount = 0;

    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familyCount, nullptr);

    std::vector<VkQueueFamilyProperties> familyPropertiesList(familyCount);

    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familyCount, familyPropertiesList.data());

    unsigned int lastCapabilityCount = 0;
    unsigned int bestCapabilityCount = preciseCapabilitiesCount > 0 ? preciseCapabilitiesCount : 4;

    for (uint32_t i = 0; i < familyCount; i++) {
        VkQueueFamilyProperties const& familyProperties = familyPropertiesList[i];

        unsigned int capabilityCount = 0;

        if (familyProperties.queueCount < 1 || (familyProperties.queueFlags & flags) != flags) {
            continue;
        }

        if (familyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            capabilityCount++;
        }

        if (familyProperties.queueFlags & VK_QUEUE_COMPUTE_BIT) {
            capabilityCount++;
        }

        if (familyProperties.queueFlags & VK_QUEUE_TRANSFER_BIT) {
            capabilityCount++;
        }

        if (familyProperties.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) {
            capabilityCount++;
        }

        if (capabilityCount == bestCapabilityCount) {
            queueFamilyIndex = i;
            break;
        }

        if (preciseCapabilitiesCount == 0 && capabilityCount > lastCapabilityCount) {
            lastCapabilityCount = capabilityCount;
            queueFamilyIndex = i;
        }
    }

    return queueFamilyIndex;
}

Device::Device(VkPhysicalDevice const& vkPhysicalDevice)
  : name_{ "" }
  , vkPhysicalDevice_{ vkPhysicalDevice }
  , vkDeviceProperties_{}
  , vkPhysicalDeviceMemoryProperties_{}
  , vkDevice_{ vkDestroyDevice }
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
