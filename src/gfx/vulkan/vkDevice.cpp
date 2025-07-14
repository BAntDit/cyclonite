//
// Created by anton on 6/26/25.
//

#include "vkDevice.h"

namespace cyclonite::gfx::vulkan {
namespace {
auto getBestQueueFamilyIndex(std::vector<VkQueueFamilyProperties> const& familyPropertiesList,
                             VkQueueFlags flags,
                             uint_fast8_t desiredCapabilitiesCount = 0) -> uint32_t
{
    auto queueFamilyIndex = std::numeric_limits<uint32_t>::max();

    auto lastCapabilityCount = uint_fast8_t{ 0 };
    auto bestCapabilityCount = desiredCapabilitiesCount > 0 ? desiredCapabilitiesCount : uint_fast8_t{ 4 };

    for (auto i = uint32_t{ 0 }, count = static_cast<uint32_t>(familyPropertiesList.size()); i < count; i++) {
        auto const& familyProperties = familyPropertiesList[i];
        auto capabilityCount = uint_fast8_t{ 0 };

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

        if (capabilityCount == bestCapabilityCount && desiredCapabilitiesCount == 0) {
            queueFamilyIndex = i;
            break;
        }

        if (desiredCapabilitiesCount == 0 && capabilityCount > lastCapabilityCount) {
            lastCapabilityCount = capabilityCount;
            queueFamilyIndex = i;
        } else if (desiredCapabilitiesCount != 0 && capabilityCount < lastCapabilityCount) {
            lastCapabilityCount = capabilityCount;
            queueFamilyIndex = i;
        }
    }

    return queueFamilyIndex;
}
}

Device::Device(core::ResourceManagerBase* resourceManager,
               core::ResourceId resourceId,
               VkInstance vkInstance,
               VkPhysicalDevice vkPhysicalDevice,
               VkPhysicalDeviceProperties const& physicalDeviceProperties,
               std::vector<const char*> const& requiredExtensions)
  : core::ResourceBase{ resourceManager, resourceId, true }
  , vkInstance_{ vkInstance }
  , vkPhysicalDevice_{ vkPhysicalDevice }
  , name_{ physicalDeviceProperties.deviceName }
  , vendor_{}
  , vkDevice_{}
{
    uint32_t familyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice_, &familyCount, nullptr);

    auto familyPropertiesList = std::vector<VkQueueFamilyProperties>(familyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice_, &familyCount, familyPropertiesList.data());

    // to refactor with array of { flag, option }
    auto graphicsQueueFamilyIndex =
      getBestQueueFamilyIndex(familyPropertiesList, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT);

    if (graphicsQueueFamilyIndex == std::numeric_limits<uint32_t>::max()) {
        throw std::runtime_error("gfx:: could not found valid graphics queue family index for device: " + name_);
    }

    auto computeQueueFamilyIndex =
      getBestQueueFamilyIndex(familyPropertiesList, VK_QUEUE_COMPUTE_BIT, uint_fast8_t{ 1 });
}
}
