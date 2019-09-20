//
// Created by bantdit on 9/5/19.
//

#include "device.h"
#include "internal/deviceCreationFunctions.h"

namespace cyclonite::vulkan {
auto Device::_extractPhysicalDeviceNameFromProperties(VkPhysicalDeviceProperties const& deviceProperties) -> std::string
{
    return internal::getDeviceName(deviceProperties);
}

auto Device::_findBestGraphicsQueueFamilyIndex(std::vector<VkQueueFamilyProperties> const& familyProperties) -> uint32_t
{
    return internal::getBestQueueFamilyIndex(familyProperties, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT);
}

auto Device::_findBestComputeQueueFamilyIndex(std::vector<VkQueueFamilyProperties> const& familyProperties,
                                              uint32_t graphicsQueueFamilyIndex) -> uint32_t
{
    return internal::testQueueCapability(familyProperties, graphicsQueueFamilyIndex, VK_QUEUE_COMPUTE_BIT)
             ? graphicsQueueFamilyIndex
             : internal::getBestQueueFamilyIndex(familyProperties, VK_QUEUE_COMPUTE_BIT, 1u);
}

auto Device::_findBestHostTransferQueueFamilyIndex(std::vector<VkQueueFamilyProperties> const& familyProperties,
                                                   uint32_t graphicsQueueFamilyIndex) -> uint32_t
{
    uint32_t deviceHostTransferQueueFamilyIndex =
      internal::getBestQueueFamilyIndex(familyProperties, VK_QUEUE_TRANSFER_BIT, 1u);

    if (deviceHostTransferQueueFamilyIndex == std::numeric_limits<uint32_t>::max()) {
        deviceHostTransferQueueFamilyIndex = graphicsQueueFamilyIndex;
    }

    return deviceHostTransferQueueFamilyIndex;
}
}
