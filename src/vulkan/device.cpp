//
// Created by bantdit on 9/5/19.
//

#include "device.h"
#include "internal/deviceCreationFunctions.h"
#include <boost/cstdfloat.hpp>

namespace cyclonite::vulkan {
Device::Device(multithreading::TaskManager const& taskManager,
               VkInstance vkInstance,
               VkPhysicalDevice const& vkPhysicalDevice,
               VkPhysicalDeviceProperties const& physicalDeviceProperties,
               std::vector<const char*> const& requiredExtensions)
  : capabilities_{ physicalDeviceProperties.limits }
  , vkInstance_{ vkInstance }
  , vkPhysicalDevice_{ vkPhysicalDevice }
  , id_{ physicalDeviceProperties.deviceID }
  , name_{ physicalDeviceProperties.deviceName }
  , vendor_{ internal::getVendorName(physicalDeviceProperties) }
  , graphicsQueueIndex_{ std::numeric_limits<size_t>::max() }
  , computeQueueIndex_{ std::numeric_limits<size_t>::max() }
  , deviceHostTransferQueueIndex_{ std::numeric_limits<size_t>::max() }
  , vkDevice_{ vkDestroyDevice }
  , queueFamilyIndices_{}
  , vkQueues_{}
  , memoryManager_{ nullptr }
  , commandPool_{ nullptr }
{
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

    if (deviceHostTransferQueueFamilyIndex == std::numeric_limits<uint32_t>::max() ||
        deviceHostTransferQueueFamilyIndex == computeQueueFamilyIndex) {
        deviceHostTransferQueueFamilyIndex = graphicsQueueFamilyIndex;
    }

    graphicsQueueIndex_ = queueFamilyIndices_.size();
    queueFamilyIndices_.push_back(graphicsQueueFamilyIndex);

    computeQueueIndex_ = computeQueueFamilyIndex == graphicsQueueFamilyIndex
                           ? graphicsQueueIndex_
                           : (queueFamilyIndices_.push_back(computeQueueFamilyIndex), queueFamilyIndices_.size() - 1);

    deviceHostTransferQueueIndex_ =
      deviceHostTransferQueueFamilyIndex == graphicsQueueFamilyIndex
        ? graphicsQueueIndex_
        : deviceHostTransferQueueFamilyIndex == computeQueueFamilyIndex
            ? computeQueueIndex_
            : (queueFamilyIndices_.push_back(deviceHostTransferQueueFamilyIndex), queueFamilyIndices_.size() - 1);

    std::array<boost::float32_t, 1> deviceQueuePriority = { 1.0f };

    std::vector<VkDeviceQueueCreateInfo> deviceQueuesCreateInfo = {};

    deviceQueuesCreateInfo.reserve(queueFamilyIndices_.size());

    for (uint32_t queueFamilyIndex : queueFamilyIndices_) {
        VkDeviceQueueCreateInfo deviceQueueInfo = {};
        deviceQueueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        deviceQueueInfo.queueCount = static_cast<uint32_t>(deviceQueuePriority.size());
        deviceQueueInfo.queueFamilyIndex = queueFamilyIndex;
        deviceQueueInfo.pQueuePriorities = deviceQueuePriority.data();

        deviceQueuesCreateInfo.push_back(deviceQueueInfo);
    }

    if (!internal::testRequiredDeviceExtensions(vkPhysicalDevice_, requiredExtensions)) {
        throw std::runtime_error("device " + name_ + " does not support required extensions");
    }

    VkPhysicalDeviceFeatures features = {};

    vkGetPhysicalDeviceFeatures(vkPhysicalDevice_, &features);

    // turn off unused features (for now)
    features.robustBufferAccess = VK_FALSE;
    features.shaderFloat64 = VK_FALSE;
    features.shaderInt64 = VK_FALSE;
    features.inheritedQueries = VK_FALSE;

    VkDeviceCreateInfo deviceInfo = {};
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.queueCreateInfoCount = static_cast<uint32_t>(deviceQueuesCreateInfo.size());
    deviceInfo.pQueueCreateInfos = deviceQueuesCreateInfo.data();
    deviceInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
    deviceInfo.ppEnabledExtensionNames = requiredExtensions.data();
    deviceInfo.pEnabledFeatures = &features;

    internal::_handleDeviceCreationResult(vkCreateDevice(vkPhysicalDevice_, &deviceInfo, nullptr, &vkDevice_));

    const uint32_t queueIndex = 0; // always create one queue per device for a while
                                   // so, queueIndex is constantly 0

    vkQueues_.reserve(queueFamilyIndices_.size());

    for (uint32_t queueFamilyIndex : queueFamilyIndices_) {
        vkQueues_.emplace_back();
        vkGetDeviceQueue(static_cast<VkDevice>(vkDevice_), queueFamilyIndex, queueIndex, &vkQueues_.back());
    }

    memoryManager_ = std::make_unique<MemoryManager>(taskManager, *this);

    commandPool_ = std::make_unique<CommandPool>(taskManager, *this);
}

auto Device::graphicsQueue() const -> VkQueue
{
    return static_cast<VkQueue>(vkQueues_[graphicsQueueIndex_]);
}

auto Device::computeQueue() const -> VkQueue
{
    return static_cast<VkQueue>(vkQueues_[computeQueueIndex_]);
}

auto Device::hostTransferQueue() const -> VkQueue
{
    return static_cast<VkQueue>(vkQueues_[deviceHostTransferQueueIndex_]);
}

auto Device::graphicsQueueFamilyIndex() const -> uint32_t
{
    return queueFamilyIndices_[graphicsQueueIndex_];
}

auto Device::computeQueueFamilyIndex() const -> uint32_t
{
    return queueFamilyIndices_[computeQueueIndex_];
}

auto Device::hostTransferQueueFamilyIndex() const -> uint32_t
{
    return queueFamilyIndices_[deviceHostTransferQueueIndex_];
}

Device::Capabilities::Capabilities(VkPhysicalDeviceLimits const& vkPhysicalDeviceLimits)
  : minMemoryMapAlignment{ vkPhysicalDeviceLimits.minMemoryMapAlignment }
  , minStorageBufferOffsetAlignment{ vkPhysicalDeviceLimits.minStorageBufferOffsetAlignment }
  , minUniformBufferOffsetAlignment{ vkPhysicalDeviceLimits.minUniformBufferOffsetAlignment }
  , minTexelBufferOffsetAlignment{ vkPhysicalDeviceLimits.minTexelBufferOffsetAlignment }
  , maxPushConstantsSize{ vkPhysicalDeviceLimits.maxPushConstantsSize }
  , maxColorAttachments{ vkPhysicalDeviceLimits.maxColorAttachments }
{}
}
