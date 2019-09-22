//
// Created by bantdit on 9/5/19.
//

#include "device.h"
#include "internal/deviceCreationFunctions.h"
#include <boost/cstdfloat.hpp>
#include <cstring>

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

void Device::_testExtensions(VkPhysicalDevice const& physicalDevice, std::vector<const char*> const& requiredExtensions)
{
    uint32_t extensionsCount = 0;

    std::vector<VkExtensionProperties> availableExtensionList = {};

    if (vkEnumerateDeviceExtensionProperties(vkPhysicalDevice_, nullptr, &extensionsCount, nullptr) != VK_SUCCESS) {
        throw std::runtime_error("could not enumerate available device extensions");
    }

    if (extensionsCount > 0) {
        availableExtensionList.resize(extensionsCount);

        if (vkEnumerateDeviceExtensionProperties(
              vkPhysicalDevice_, nullptr, &extensionsCount, availableExtensionList.data()) != VK_SUCCESS) {
            throw std::runtime_error("count not read properties of available extensions for device");
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
            throw std::runtime_error("required device extension: " + std::string(*extIt) + " is not supported");
        }
    }
}

void Device::_handleDeviceCreationResult(VkResult result)
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
