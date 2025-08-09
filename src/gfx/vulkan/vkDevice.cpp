//
// Created by anton on 6/26/25.
//

#include "vkDevice.h"
#include "core/hashTable.h"
#include "core/resourceRef.h"
#include "vkException.h"
#include <array>
#include <cstring>
#include <limits>
#include <metrix/enum.h>
#include <stdexcept>
#include <tuple>
#include <utility>

#if defined(GFX_DRIVER_VULKAN)

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

    for (auto* reqExtName : requiredExtensions) {
        auto it = availableExtensionList.cbegin();
        while (it != availableExtensionList.cend()) {
            if (0 == strcmp(reqExtName, (*it).extensionName))
                break;
            it++;
        }

        if (it == availableExtensionList.cend()) {
            return false;
        }
    }

    return true;
}

auto getVendorById(uint32_t id) -> DeviceVendor
{
    auto result = DeviceVendor::Unknown;

    switch (id) {
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

using queue_family_flag_req_t = core::StaticHashTable<uint32_t, 32, uint32_t>;

enum class QueueSelectionPolicy : uint32_t
{
    Required,
    BetterInclude,
    BetterExclude
};

template<size_t N>
auto getBestQueueFamilyIndex(std::vector<VkQueueFamilyProperties> const& familyProperties,
                             queue_family_flag_req_t& usage,
                             std::array<std::pair<VkQueueFlagBits, QueueSelectionPolicy>, N>&& requirements)
  -> std::tuple<uint32_t, uint32_t, VkQueueFlags>
{
    auto flags = VkQueueFlags{};
    auto queueFamilyIndex = std::numeric_limits<uint32_t>::max();
    auto queueIndex = uint32_t{ 0 };

    auto bestCapabilityCount = uint32_t{ 0 };

    for (auto i = uint32_t{ 0 }, count = static_cast<uint32_t>(familyProperties.size()); i < count; i++) {
        auto&& properties = familyProperties[i];

        auto it = usage.find(i);
        if (it != usage.end()) {
            auto [_, v] = *it;
            if (v >= properties.queueCount) {
                continue;
            }
        }

        flags = VkQueueFlags{};
        auto lastCapabilityCount = uint32_t{ 0 };
        auto skipFamily = false;
        for (auto&& [flag, policy] : requirements) {
            if (policy == QueueSelectionPolicy::Required && ((properties.queueFlags & flag) == 0)) {
                skipFamily = true;
                break;
            } else {
                flags = flags | flag;
                lastCapabilityCount++;
            }

            if (policy == QueueSelectionPolicy::BetterInclude && ((properties.queueFlags & flag) != 0)) {
                lastCapabilityCount++;
                flags = flags | flag;
            }

            if (policy == QueueSelectionPolicy::BetterExclude && ((properties.queueFlags & flag) == 0)) {
                lastCapabilityCount++;
            }
        }

        if (skipFamily)
            continue;

        if (lastCapabilityCount > bestCapabilityCount) {
            bestCapabilityCount = lastCapabilityCount;
            queueFamilyIndex = i;
        }
    }

    if (queueFamilyIndex != std::numeric_limits<uint32_t>::max()) {
        auto it = usage.find(queueFamilyIndex);
        if (it == usage.end()) {
            usage.add(queueIndex + 1, queueFamilyIndex);
        } else {
            auto& [_, v] = *it;
            assert(v < familyProperties[queueFamilyIndex].queueCount);
            queueIndex = v++;
        }
    }

    return std::tuple{ queueFamilyIndex, queueIndex, flags };
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
  , vendor_{ getVendorById(physicalDeviceProperties.deviceID) }
  , limits_{}
  , vkDevice_{ vkDestroyDevice }
  , graphicsQueue_{}
  , transferQueue_{}
  , computeQueue_{}
  , vmaAllocator_{ VK_NULL_HANDLE }
{
    if (!testRequiredDeviceExtensions(vkPhysicalDevice, requiredExtensions)) {
        throw std::runtime_error("gfx:: physical device does not supports required extensions. Device name: " + name_);
    }

    uint32_t familyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice_, &familyCount, nullptr);

    auto familyPropertiesList = std::vector<VkQueueFamilyProperties>(familyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice_, &familyCount, familyPropertiesList.data());

    auto familyQueueUsages = queue_family_flag_req_t{};

    auto [graphicsQueueFamilyIndex, graphicsQueueIndex, graphicsQueueFlags] =
      getBestQueueFamilyIndex(familyPropertiesList,
                              familyQueueUsages,
                              std::array{ std::pair{ VK_QUEUE_GRAPHICS_BIT, QueueSelectionPolicy::Required },
                                          std::pair{ VK_QUEUE_TRANSFER_BIT, QueueSelectionPolicy::Required },
                                          std::pair{ VK_QUEUE_COMPUTE_BIT, QueueSelectionPolicy::BetterInclude } });

    auto [transferQueueFamilyIndex, transferQueueIndex, transferQueueFlags] =
      getBestQueueFamilyIndex(familyPropertiesList,
                              familyQueueUsages,
                              std::array{ std::pair{ VK_QUEUE_GRAPHICS_BIT, QueueSelectionPolicy::BetterExclude },
                                          std::pair{ VK_QUEUE_TRANSFER_BIT, QueueSelectionPolicy::Required },
                                          std::pair{ VK_QUEUE_COMPUTE_BIT, QueueSelectionPolicy::BetterExclude } });

    auto [computeQueueFamilyIndex, computeQueueIndex, computeQueueFlags] =
      getBestQueueFamilyIndex(familyPropertiesList,
                              familyQueueUsages,
                              std::array{ std::pair{ VK_QUEUE_GRAPHICS_BIT, QueueSelectionPolicy::BetterExclude },
                                          std::pair{ VK_QUEUE_TRANSFER_BIT, QueueSelectionPolicy::BetterInclude },
                                          std::pair{ VK_QUEUE_COMPUTE_BIT, QueueSelectionPolicy::Required } });

    if (graphicsQueueFamilyIndex == std::numeric_limits<uint32_t>::max()) {
        throw std::runtime_error("gfx:: could not find valid graphics queue family index for device: " + name_);
    }

    assert(transferQueueIndex < 3);

    auto queueCreateInfoCount = uint32_t{ 0 };
    auto deviceQueueCreateInfoArray = std::array<VkDeviceQueueCreateInfo, 3>{};

    auto graphicsQueuePriorities = std::array<float, 3>{ 1.0f, 1.0f, 1.0f };
    if (graphicsQueueFamilyIndex == transferQueueFamilyIndex) {
        graphicsQueuePriorities[transferQueueIndex] = 0.5f;
    }

    auto computeQueuePriorities = std::array<float, 3>{ 1.0f, 1.0f, 1.0f };
    if (computeQueueFamilyIndex == transferQueueFamilyIndex) {
        computeQueuePriorities[transferQueueIndex] = 0.5f;
    }

    auto transferQueuePriorities = std::array<float, 3>{ 1.0f, 1.0f, 1.0f };
    transferQueuePriorities[transferQueueIndex] = 0.5f;

    // graphics
    {
        auto it = familyQueueUsages.find(graphicsQueueFamilyIndex);
        assert(it != familyQueueUsages.end());

        auto const [_, usageCount] = *it;
        assert(usageCount > 0);
        assert(usageCount <= 3);

        familyQueueUsages.remove(it);

        auto& deviceQueueCreateInfo = deviceQueueCreateInfoArray[queueCreateInfoCount++];
        deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        deviceQueueCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex;
        deviceQueueCreateInfo.queueCount = usageCount;
        deviceQueueCreateInfo.pQueuePriorities = graphicsQueuePriorities.data();
    }

    // transfer
    if (transferQueueFamilyIndex != std::numeric_limits<uint32_t>::max()) {
        limits_.dedicatedTransferQueue = true;
        if (auto it = familyQueueUsages.find(transferQueueFamilyIndex); it != familyQueueUsages.end()) {
            auto const [_, usageCount] = *it;
            assert(usageCount > 0);
            assert(usageCount <= 3);

            familyQueueUsages.remove(it);

            auto& deviceQueueCreateInfo = deviceQueueCreateInfoArray[queueCreateInfoCount++];
            deviceQueueCreateInfo.queueFamilyIndex = transferQueueFamilyIndex;
            deviceQueueCreateInfo.queueCount = usageCount;
            deviceQueueCreateInfo.pQueuePriorities = transferQueuePriorities.data();
        }
    }

    // compute
    if (computeQueueFamilyIndex != std::numeric_limits<uint32_t>::max()) {
        limits_.supportCompute = true;
        limits_.dedicatedComputeQueue = true;

        if (auto it = familyQueueUsages.find(computeQueueFamilyIndex); it != familyQueueUsages.end()) {
            auto const [_, usageCount] = *it;
            assert(usageCount > 0);
            assert(usageCount <= 3);

            familyQueueUsages.remove(it);

            auto& deviceQueueCreateInfo = deviceQueueCreateInfoArray[queueCreateInfoCount++];
            deviceQueueCreateInfo.queueFamilyIndex = computeQueueFamilyIndex;
            deviceQueueCreateInfo.queueCount = usageCount;
            deviceQueueCreateInfo.pQueuePriorities = computeQueuePriorities.data();
        }
    } else {
        limits_.supportCompute = ((graphicsQueueFlags & VK_QUEUE_COMPUTE_BIT) != 0);
    }

    auto features = VkPhysicalDeviceFeatures{};

    // turn off unused features (for now)
    features.robustBufferAccess = VK_FALSE;
    features.shaderFloat64 = VK_FALSE;
    features.shaderInt64 = VK_FALSE;
    features.inheritedQueries = VK_FALSE;

    auto deviceInfo = VkDeviceCreateInfo{};
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.queueCreateInfoCount = queueCreateInfoCount;
    deviceInfo.pQueueCreateInfos = deviceQueueCreateInfoArray.data();
    deviceInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
    deviceInfo.ppEnabledExtensionNames = requiredExtensions.data();
    deviceInfo.pEnabledFeatures = &features;

    if (auto vkResult = vkCreateDevice(vkPhysicalDevice_, &deviceInfo, nullptr, &vkDevice_); vkResult != VK_SUCCESS) {
        throw Exception{ vkResult, "vkCreateDevice" };
    }

    assert(graphicsQueueFamilyIndex != std::numeric_limits<uint32_t>::max());
    vkGetDeviceQueue(static_cast<VkDevice>(vkDevice_), graphicsQueueFamilyIndex, graphicsQueueIndex, &graphicsQueue_);

    if (transferQueueFamilyIndex != std::numeric_limits<uint32_t>::max()) {
        vkGetDeviceQueue(
          static_cast<VkDevice>(vkDevice_), transferQueueFamilyIndex, transferQueueIndex, &transferQueue_);
    }

    if (computeQueueFamilyIndex != std::numeric_limits<uint32_t>::max()) {
        vkGetDeviceQueue(static_cast<VkDevice>(vkDevice_), computeQueueFamilyIndex, computeQueueIndex, &computeQueue_);
    }

    auto allocatorCreateInfo = VmaAllocatorCreateInfo{};
    allocatorCreateInfo.flags = VMA_ALLOCATOR_CREATE_EXTERNALLY_SYNCHRONIZED_BIT;
    allocatorCreateInfo.physicalDevice = vkPhysicalDevice_;
    allocatorCreateInfo.device = static_cast<VkDevice>(vkDevice_);
    allocatorCreateInfo.instance = vkInstance;
    allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_0;

    if (auto vkResult = vmaCreateAllocator(&allocatorCreateInfo, &vmaAllocator_); vkResult != VK_SUCCESS) {
        throw Exception{ vkResult, "vmaCreateAllocator" };
    }

    limits_.maxColorAttachmentCount = static_cast<uint8_t>(physicalDeviceProperties.limits.maxColorAttachments);
}

auto Device::createSurface(uint32_t width, uint32_t height, std::string_view title, SurfaceFlagBits flags)
  -> core::ResourceRef
{
    // TODO::
}

Device::~Device()
{
    assert(vmaAllocator_ != VK_NULL_HANDLE);
    vmaDestroyAllocator(vmaAllocator_);
}
}

#endif // GFX_DRIVER_VULKAN
