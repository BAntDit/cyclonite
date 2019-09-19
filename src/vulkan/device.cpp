//
// Created by bantdit on 9/5/19.
//

#include "device.h"
#include "internal/deviceCreationFunctions.h"

namespace cyclonite::vulkan {
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

    name_ = internal::getDeviceName(vkDeviceProperties_);

    uint32_t familyCount = 0;

    vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice_, &familyCount, nullptr);

    std::vector<VkQueueFamilyProperties> familyPropertiesList(familyCount);

    vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice_, &familyCount, familyPropertiesList.data());

    uint32_t graphicsQueueFamilyIndex =
      internal::getBestQueueFamilyIndex(familyPropertiesList, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT);

    if (graphicsQueueFamilyIndex == std::numeric_limits<uint32_t>::max()) {
        throw std::runtime_error("could not found valid graphics queue family index for device: " + name_);
    }

    uint32_t computeQueueFamilyIndex =
      internal::testQueueCapability(familyPropertiesList, graphicsQueueFamilyIndex, VK_QUEUE_COMPUTE_BIT)
        ? graphicsQueueFamilyIndex
        : internal::getBestQueueFamilyIndex(familyPropertiesList, VK_QUEUE_COMPUTE_BIT, 1u);

    if (computeQueueFamilyIndex == std::numeric_limits<uint32_t>::max()) {
        throw std::runtime_error("could not found valid compute queue family index for device: " + name_);
    }

    uint32_t deviceHostTransferQueueFamilyIndex =
      internal::getBestQueueFamilyIndex(familyPropertiesList, VK_QUEUE_TRANSFER_BIT, 1u);

    if (deviceHostTransferQueueFamilyIndex == std::numeric_limits<uint32_t>::max()) {
        deviceHostTransferQueueFamilyIndex = graphicsQueueFamilyIndex;
    }

    // todo:: presentation queue family index

    VkPhysicalDeviceFeatures features = {};

    vkGetPhysicalDeviceFeatures(vkPhysicalDevice_, &features);

    // turn off unused features (for now)
    features.robustBufferAccess = VK_FALSE;
    features.shaderFloat64 = VK_FALSE;
    features.shaderInt64 = VK_FALSE;
    features.inheritedQueries = VK_FALSE;
}
}
