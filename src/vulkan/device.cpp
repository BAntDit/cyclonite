//
// Created by bantdit on 9/5/19.
//

#include "device.h"
#include "internal/deviceCreationFunctions.h"
#include <boost/cstdfloat.hpp>

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

void Device::_defineQueuesInfo(std::vector<uint32_t> const& queueFamilyIndices,
                               std::vector<VkDeviceQueueCreateInfo>& deviceQueuesCreateInfo)
{
    std::array<boost::float32_t, 1> deviceQueuePriority = { 1.0f };

    deviceQueuesCreateInfo.reserve(queueFamilyIndices.size());

    for (uint32_t queueFamilyIndex : queueFamilyIndices) {
        VkDeviceQueueCreateInfo deviceQueueInfo = {};
        deviceQueueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        deviceQueueInfo.queueCount = static_cast<uint32_t>(deviceQueuePriority.size());
        deviceQueueInfo.queueFamilyIndex = queueFamilyIndex;
        deviceQueueInfo.pQueuePriorities = deviceQueuePriority.data();

        deviceQueuesCreateInfo.push_back(deviceQueueInfo);
    }
}

void Device::_defineQueueIndices(std::vector<uint32_t>& queueFamilyIndices,
                                 uint32_t graphicsQueueFamilyIndex,
                                 uint32_t computeQueueFamilyIndex,
                                 uint32_t hostTransferQueueFamilyIndex,
                                 uint32_t presentationQueueFamilyIndex)
{
    graphicsQueueIndex_ = queueFamilyIndices.size();
    queueFamilyIndices.push_back(graphicsQueueFamilyIndex);

    computeQueueIndex_ = computeQueueFamilyIndex == graphicsQueueFamilyIndex
                           ? graphicsQueueIndex_
                           : (queueFamilyIndices.push_back(computeQueueFamilyIndex), queueFamilyIndices.size() - 1);

    deviceHostTransferQueueIndex_ =
      hostTransferQueueFamilyIndex == graphicsQueueFamilyIndex
        ? graphicsQueueIndex_
        : hostTransferQueueFamilyIndex == computeQueueFamilyIndex
            ? computeQueueIndex_
            : (queueFamilyIndices.push_back(hostTransferQueueFamilyIndex), queueFamilyIndices.size() - 1);

    if (presentationQueueFamilyIndex == std::numeric_limits<uint32_t>::max())
        return;

    presentationQueueIndex_ =
      presentationQueueFamilyIndex == graphicsQueueFamilyIndex
        ? graphicsQueueIndex_
        : presentationQueueFamilyIndex == computeQueueFamilyIndex
            ? computeQueueIndex_
            : (queueFamilyIndices.push_back(presentationQueueFamilyIndex), queueFamilyIndices.size() - 1);
}
}
