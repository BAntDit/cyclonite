//
// Created by bantdit on 9/5/19.
//

#include "device.h"

namespace cyclonite::vulkan {
static uint32_t getBestQueueFamilyIndex(std::vector<VkQueueFamilyProperties> const& familyPropertiesList,
                                        VkQueueFlags flags,
                                        unsigned int desiredCapabilitiesCount = 0)
{
    uint32_t queueFamilyIndex = std::numeric_limits<uint32_t>::max();

    unsigned int lastCapabilityCount = 0;
    unsigned int bestCapabilityCount = desiredCapabilitiesCount > 0 ? desiredCapabilitiesCount : 4;

    for (uint32_t i = 0; i < static_cast<uint32_t>(familyPropertiesList.size()); i++) {
        auto const& familyProperties = familyPropertiesList[i];

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

        if (desiredCapabilitiesCount == 0 && capabilityCount > lastCapabilityCount) {
            lastCapabilityCount = capabilityCount;
            queueFamilyIndex = i;
        } else if (desiredCapabilitiesCount != 0 && capabilityCount < lastCapabilityCount) {
        }
    }

    return queueFamilyIndex;
}

static bool testQueueCapability(std::vector<VkQueueFamilyProperties> const& familyPropertiesList,
                         uint32_t queueFamilyIndex,
                         const VkQueueFlags& capabilityFlags)
{
    auto const& familyProperties = familyPropertiesList[static_cast<size_t>(queueFamilyIndex)];

    return (familyProperties.queueFlags & capabilityFlags) == capabilityFlags;
}

Device::Device(VkPhysicalDevice const& vkPhysicalDevice)
  : name_{ "" }
  , graphicsQueueIndex_{ std::numeric_limits<size_t>::max() }
  , computeQueueIndex_{ std::numeric_limits<size_t>::max() }
  , deviceHostTransferIndex_{ std::numeric_limits<size_t>::max() }
  , presentationQueueIndex_{ std::numeric_limits<size_t>::max() }
  , vkPhysicalDevice_{ vkPhysicalDevice }
  , vkDeviceProperties_{}
  , vkPhysicalDeviceMemoryProperties_{}
  , vkDevice_{ vkDestroyDevice }
{
    vkGetPhysicalDeviceProperties(vkPhysicalDevice_, &vkDeviceProperties_);
    vkGetPhysicalDeviceMemoryProperties(vkPhysicalDevice_, &vkPhysicalDeviceMemoryProperties_);

    uint32_t familyCount = 0;

    vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice_, &familyCount, nullptr);

    std::vector<VkQueueFamilyProperties> familyPropertiesList(familyCount);

    vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice_, &familyCount, familyPropertiesList.data());

    uint32_t graphicsQueueFamilyIndex =
      getBestQueueFamilyIndex(familyPropertiesList, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT);

    if (graphicsQueueFamilyIndex == std::numeric_limits<uint32_t>::max()) {
        throw std::runtime_error("could not found valid graphics queue family index for device: " + name_);
    }

    uint32_t computeQueueFamilyIndex =
      testQueueCapability(familyPropertiesList, graphicsQueueFamilyIndex, VK_QUEUE_COMPUTE_BIT)
        ? graphicsQueueFamilyIndex
        : getBestQueueFamilyIndex(familyPropertiesList, VK_QUEUE_COMPUTE_BIT, 1u);

    if (computeQueueFamilyIndex == std::numeric_limits<uint32_t>::max()) {
        throw std::runtime_error("could not found valid compute queue family index for device: " + name_);
    }

    uint32_t deviceHostTransferQueueFamilyIndex =
      getBestQueueFamilyIndex(familyPropertiesList, VK_QUEUE_TRANSFER_BIT, 1u);

    if (deviceHostTransferQueueFamilyIndex == std::numeric_limits<uint32_t>::max()) {
        deviceHostTransferQueueFamilyIndex = graphicsQueueFamilyIndex;
    }

    // todo:: presentation queue family index

    VkPhysicalDeviceFeatures features = {};

    // turn off unused features (for now)
    features.robustBufferAccess = VK_FALSE;
    features.shaderFloat64 = VK_FALSE;
    features.shaderInt64 = VK_FALSE;
    features.inheritedQueries = VK_FALSE;
}
}
