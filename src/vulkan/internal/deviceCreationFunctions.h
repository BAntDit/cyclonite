//
// Created by bantdit on 9/8/19.
//

#include <sstream>
#include <vulkan/vulkan.h>

#ifndef CYCLONITE_DEVICECREATIONFUNCTIONS_H
#define CYCLONITE_DEVICECREATIONFUNCTIONS_H

namespace cyclonite::vulkan::internal {
static std::string getDeviceName(VkPhysicalDeviceProperties const& physicalDeviceProperties)
{
    std::ostringstream oss;

    switch (physicalDeviceProperties.deviceType) {
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            oss << "Integrated GPU ";
            break;
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            oss << "Discrete GPU ";
            break;
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
            oss << "Virtual GPU ";
            break;
        case VK_PHYSICAL_DEVICE_TYPE_CPU:
            oss << "CPU ";
            break;
        default:
            oss << "unknown device ";
    }

    switch (physicalDeviceProperties.vendorID) {
        case 0x1002:
            oss << "AMD ";
            break;
        case 0x1010:
            oss << "ImgTec ";
            break;
        case 0x10DE:
            oss << "NVIDIA ";
            break;
        case 0x13B5:
            oss << "ARM ";
            break;
        case 0x5143:
            oss << "Qualcomm ";
            break;
        case 0x8086:
            oss << "INTEL ";
            break;
        default:
            oss << "";
    }

    oss << physicalDeviceProperties.deviceName;

    oss << " (id: " << physicalDeviceProperties.deviceID << ")";

    return oss.str();
}

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
}

#endif // CYCLONITE_DEVICECREATIONFUNCTIONS_H
