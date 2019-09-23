//
// Created by bantdit on 9/8/19.
//

#include <cstring>
#include <vulkan/vulkan.h>

#ifndef CYCLONITE_DEVICECREATIONFUNCTIONS_H
#define CYCLONITE_DEVICECREATIONFUNCTIONS_H

namespace cyclonite::vulkan::internal {
static auto getVendorName(VkPhysicalDeviceProperties const& physicalDeviceProperties) -> std::string
{
    std::string result{};

    switch (physicalDeviceProperties.vendorID) {
        case 0x1002:
            result = u8"AMD";
            break;
        case 0x1010:
            result = u8"ImgTec";
            break;
        case 0x10DE:
            result = u8"NVIDIA";
            break;
        case 0x13B5:
            result = u8"ARM";
            break;
        case 0x5143:
            result = u8"Qualcomm";
            break;
        case 0x8086:
            result = u8"INTEL";
            break;
        default:
            result = u8"unknown";
    }

    return result;
}

static auto getBestQueueFamilyIndex(std::vector<VkQueueFamilyProperties> const& familyPropertiesList,
                                    VkQueueFlags flags,
                                    unsigned int desiredCapabilitiesCount = 0) -> uint32_t
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
            lastCapabilityCount = capabilityCount;
            queueFamilyIndex = i;
        }
    }

    return queueFamilyIndex;
}

static auto testQueueCapability(std::vector<VkQueueFamilyProperties> const& familyPropertiesList,
                                uint32_t queueFamilyIndex,
                                const VkQueueFlags& capabilityFlags) -> bool
{
    auto const& familyProperties = familyPropertiesList[static_cast<size_t>(queueFamilyIndex)];

    return (familyProperties.queueFlags & capabilityFlags) == capabilityFlags;
}

static auto testRequiredDeviceExtensions(VkPhysicalDevice const& physicalDevice,
                                         std::vector<const char*> const& requiredExtensions) -> bool
{
    uint32_t extensionsCount = 0;

    std::vector<VkExtensionProperties> availableExtensionList = {};

    if (vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionsCount, nullptr) != VK_SUCCESS) {
        return false;
    }

    if (extensionsCount > 0) {
        availableExtensionList.resize(extensionsCount);

        if (vkEnumerateDeviceExtensionProperties(
              physicalDevice, nullptr, &extensionsCount, availableExtensionList.data()) != VK_SUCCESS) {
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

static void _handleDeviceCreationResult(VkResult result)
{
    if (result == VK_SUCCESS)
        return;

    switch (result) {
        case VK_ERROR_OUT_OF_HOST_MEMORY:
            throw std::runtime_error("running out of system memory on attempt to create logical device");
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            throw std::runtime_error("running out of device memory on attempt to create logical device");
        case VK_ERROR_INITIALIZATION_FAILED:
            throw std::runtime_error("initialization failed on attempt to create logical device");
        case VK_ERROR_EXTENSION_NOT_PRESENT:
            throw std::runtime_error("extension is not present on device");
        case VK_ERROR_FEATURE_NOT_PRESENT:
            throw std::runtime_error("feature not presented");
        case VK_ERROR_TOO_MANY_OBJECTS:
            throw std::runtime_error("too many object, device creation failed");
        case VK_ERROR_DEVICE_LOST:
            throw std::runtime_error("device lost on attempt to create device");
        default:
            assert(false);
    }

    throw std::runtime_error("could not create logical device");
}
}

#endif // CYCLONITE_DEVICECREATIONFUNCTIONS_H
