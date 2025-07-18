//
// Created by anton on 6/26/25.
//

#include "vkDevice.h"
#include <array>
#include <cstring>
#include <stdexcept>
#include <utility>
#include <metrix/enum.h>

namespace cyclonite::gfx::vulkan {
namespace {
auto testRequiredDeviceExtensions(VkPhysicalDevice physicalDevice, std::vector<const char*> const& requiredExtensions)
{
    auto extensionCount = uint32_t{ 0 };
    auto availableExtensionList = std::vector<VkExtensionProperties>{};

    if (vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr) != VK_SUCCESS) {
        return false;
    }

    if (extensionCount > 0) {
        availableExtensionList.resize(extensionCount);

        if (vkEnumerateDeviceExtensionProperties(
              physicalDevice, nullptr, &extensionCount, availableExtensionList.data()) != VK_SUCCESS) {
            return false;
        }
    }

    for (auto extIt = requiredExtensions.cbegin(); extIt != requiredExtensions.cend(); ++extIt) {
        auto it = availableExtensionList.cbegin();

        while (it != availableExtensionList.cend()) {
            if (0 == strcmp((*extIt), (*it).extensionName))
                break;
            it++;
        }

        if (it == availableExtensionList.cend()) {
            return false;
        }
    }

    return true;
}

auto getVendoById(uint32_t id) -> DeviceVendor {
    auto result = DeviceVendor::Unknown;
    
    switch (id) 
    { 
        case metrix::value_cast(DeviceVendor::AMD):
            result = DeviceVendor::AMD;
            break;
        case metrix::value_cast(DeviceVendor::NVIDIA):
            result = DeviceVendor::NVIDIA;
            break;
        case metrix::value_cast(DeviceVendor::ImgTec_PowerVR):
            result = DeviceVendor::ImgTec_PowerVR;
            break;
        case metrix::value_cast(DeviceVendor::ARM_MaliGPU):
            result = DeviceVendor::ARM_MaliGPU;
            break;
        case metrix::value_cast(DeviceVendor::Intel):
            result = DeviceVendor::Intel;
            break;
        case metrix::value_cast(DeviceVendor::Google_SwiftShader_virtualGPU):
            result = DeviceVendor::Google_SwiftShader_virtualGPU;
            break;
        case metrix::value_cast(DeviceVendor::Apple):
            result = DeviceVendor::Apple;
            break;
        case metrix::value_cast(DeviceVendor::QEMU_emulatedGPU):
            result = DeviceVendor::QEMU_emulatedGPU;
            break;
        case metrix::value_cast(DeviceVendor::Qualcomm_AdrenoGPU):
            result = DeviceVendor::Qualcomm_AdrenoGPU;
            break;
        case metrix::value_cast(DeviceVendor::VIATechnologies):
            result = DeviceVendor::VIATechnologies;
            break;
        case metrix::value_cast(DeviceVendor::Vivante):
            result = DeviceVendor::Vivante;
            break;
        case metrix::value_cast(DeviceVendor::VMware_virtualGPU):
            result = DeviceVendor::VMware_virtualGPU;
            break;
        default:
            result = DeviceVendor::Unknown;
    }

    return result;
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
  , vendor_{ getVendoById(physicalDeviceProperties.deviceID) }
  , limits_{}
  , vkDevice_{}
{
    if (!testRequiredDeviceExtensions(vkPhysicalDevice, requiredExtensions)) {
        throw std::runtime_error("gfx:: physical device does not supports required extensions.");
    }

    uint32_t familyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice_, &familyCount, nullptr);

    auto familyPropertiesList = std::vector<VkQueueFamilyProperties>(familyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice_, &familyCount, familyPropertiesList.data());

    auto queueFamilyCount = uint32_t{ 0 };
    auto graphicsQueueFamilyIndex = std::numeric_limits<uint32_t>::max();
    auto graphicsQueueIndex = std::numeric_limits<uint32_t>::max();
    
    {
        auto bestCapabilityCount = uint32_t{ 0 };
        
        for (auto i = size_t{ 0 }, count = familyPropertiesList.size(); i < count; i++) {
            auto lastCapabilityCount = uint32_t{ 0 };
            auto& properties = familyPropertiesList[i];
            auto requiredFlags = (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT);
            
            if ((properties.queueFlags & requiredFlags) == requiredFlags) {
                lastCapabilityCount++;
            } else {
                continue;
            }

            if ((properties.queueFlags & VK_QUEUE_COMPUTE_BIT) != 0) {
                lastCapabilityCount++;
            }

            if (lastCapabilityCount > bestCapabilityCount) {
                graphicsQueueFamilyIndex = i;
                bestCapabilityCount = lastCapabilityCount;
            }
        }

        if (graphicsQueueFamilyIndex == std::numeric_limits<uint32_t>::max()) {
            throw std::runtime_error("gfx:: could not found valid graphics queue family index for device: " + name_);
        } else {
            graphicsQueueIndex = 0;
            queueFamilyCount++;
        }
    }

    auto transferQueueFamilyIndex = std::numeric_limits<uint32_t>::max();
    auto transferQueueIndex = std::numeric_limits<uint32_t>::max();

    {
        auto bestCapabilityCount = uint32_t{ 0 };

        for (auto i = size_t{ 0 }, count = familyPropertiesList.size(); i < count; i++) {
        
        }
    }

    /*
    auto graphicsQueueRequirements = std::array{ std::pair{ VK_QUEUE_GRAPHICS_BIT, QueueFlagRequirements::Required },
                                                 std::pair{ VK_QUEUE_TRANSFER_BIT, QueueFlagRequirements::Required },
                                                 std::pair{ VK_QUEUE_COMPUTE_BIT, QueueFlagRequirements::Optional },
                                                 std::pair{ VK_QUEUE_SPARSE_BINDING_BIT, QueueFlagRequirements::Optional} };

    auto graphicsQueueFamilyIndex = getBestQueueFamilyIndex(familyPropertiesList, graphicsQueueRequirements);
    if (graphicsQueueFamilyIndex == std::numeric_limits<uint32_t>::max()) {
        throw std::runtime_error("gfx:: could not found valid graphics queue family index for device: " + name_);
    }

    auto transferQueueRequirements =
      std::array{ std::pair{ VK_QUEUE_GRAPHICS_BIT, QueueFlagRequirements::Unused },
                  std::pair{ VK_QUEUE_TRANSFER_BIT, QueueFlagRequirements::Required },
                  std::pair{ VK_QUEUE_COMPUTE_BIT, QueueFlagRequirements::Unused },
                  std::pair{ VK_QUEUE_SPARSE_BINDING_BIT, QueueFlagRequirements::Unused } };

    auto transferQueueFamilyIndex = getBestQueueFamilyIndex(familyPropertiesList, transferQueueRequirements);
    if (transferQueueFamilyIndex == std::numeric_limits<uint32_t>::max()) {
        throw std::runtime_error("gfx:: could not found valid transfer queue family index for device: " + name_);
    }
    */
    // to refactor with array of { flag, option }
    /*auto graphicsQueueFamilyIndex =
      getBestQueueFamilyIndex(familyPropertiesList, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT);

    if (graphicsQueueFamilyIndex == std::numeric_limits<uint32_t>::max()) {
        throw std::runtime_error("gfx:: could not found valid graphics queue family index for device: " + name_);
    }

    auto computeQueueFamilyIndex =
      getBestQueueFamilyIndex(familyPropertiesList, VK_QUEUE_COMPUTE_BIT, uint_fast8_t{ 1 });*/

    auto features = VkPhysicalDeviceFeatures{};

    // turn off unused features (for now)
    features.robustBufferAccess = VK_FALSE;
    features.shaderFloat64 = VK_FALSE;
    features.shaderInt64 = VK_FALSE;
    features.inheritedQueries = VK_FALSE;

    auto deviceInfo = VkDeviceCreateInfo{};
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    // deviceInfo.queueCreateInfoCount = static_cast<uint32_t>(deviceQueuesCreateInfo.size());
    // deviceInfo.pQueueCreateInfos = deviceQueuesCreateInfo.data();
    deviceInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
    deviceInfo.ppEnabledExtensionNames = requiredExtensions.data();
    deviceInfo.pEnabledFeatures = &features;
}
}
