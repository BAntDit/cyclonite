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
enum class QueueFlagRequirements : uint8_t
{
    Required,
    Optional,
    Unused
};

template<size_t N>
auto getBestQueueFamilyIndex(std::vector<VkQueueFamilyProperties> const& familyPropertiesList,
                             std::array<std::pair<VkQueueFlagBits, QueueFlagRequirements>, N> requirements) -> uint32_t
{
    auto queueFamilyIndex = std::numeric_limits<uint32_t>::max();
    auto lastCompatibilityCount = uint_fast8_t{ 0 };

    for (auto i = uint32_t{ 0 }, count = static_cast<uint32_t>(familyPropertiesList.size()); i < count; i++) {
        auto const& familyProperties = familyPropertiesList[i];
        auto lastCompatibilityCount = uint_fast8_t{ 0 };
        auto compatibility = true;

        for (auto [flag, req] : requirements) {
            auto compatibilityCount = uint_fast8_t{ 0 };
            if ((familyProperties.queueFlags & flag) == 0 && req == QueueFlagRequirements::Required) {
                compatibility = false;
                break;
            } else if ((familyProperties.queueFlags & flag) != 0 &&
                       (req == QueueFlagRequirements::Optional || req == QueueFlagRequirements::Required)) {
                compatibilityCount++;
            } else if ((familyProperties.queueFlags & flag) == 0 && req == QueueFlagRequirements::Unused) {
                compatibilityCount++;
            }
        }

        if (!compatibility)
            continue;

        if ((compatibilityCount > lastCompatibilityCount) && compatibilityCount <= N) {
            lastCompatibilityCount = compatibilityCount;
            queueFamilyIndex = i;
        }
    }

    return queueFamilyIndex;
}

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
        case metrix::value_cast(DeviceVendor::NVIDIA):
            result = DeviceVendor::NVIDIA;
        case metrix::value_cast(DeviceVendor::ImgTec_PowerVR):
            result = DeviceVendor::ImgTec_PowerVR;
        case metrix::value_cast(DeviceVendor::ARM_MaliGPU):
            result = DeviceVendor::ARM_MaliGPU;
        case metrix::value_cast(DeviceVendor::Intel):
            result = DeviceVendor::Intel;
        case metrix::value_cast(DeviceVendor::Google_SwiftShader_virtualGPU):
            result = DeviceVendor::Google_SwiftShader_virtualGPU;
        case metrix::value_cast(DeviceVendor::Apple):
            result = DeviceVendor::Apple;
        case metrix::value_cast(DeviceVendor::QEMU_emulatedGPU):
            result = DeviceVendor::QEMU_emulatedGPU;
        case metrix::value_cast(DeviceVendor::Qualcomm_AdrenoGPU):
            result = DeviceVendor::Qualcomm_AdrenoGPU;
        case metrix::value_cast(DeviceVendor::VIATechnologies):
            result = DeviceVendor::VIATechnologies;
        case metrix::value_cast(DeviceVendor::Vivante):
            result = DeviceVendor::Vivante;
        case metrix::value_cast(DeviceVendor::VMware_virtualGPU):
            result = DeviceVendor::VMware_virtualGPU;
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

    // to refactor with array of { flag, option }
    /*auto graphicsQueueFamilyIndex =
      getBestQueueFamilyIndex(familyPropertiesList, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT);

    if (graphicsQueueFamilyIndex == std::numeric_limits<uint32_t>::max()) {
        throw std::runtime_error("gfx:: could not found valid graphics queue family index for device: " + name_);
    }

    auto computeQueueFamilyIndex =
      getBestQueueFamilyIndex(familyPropertiesList, VK_QUEUE_COMPUTE_BIT, uint_fast8_t{ 1 });*/
}
}
