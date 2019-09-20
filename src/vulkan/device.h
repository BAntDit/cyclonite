//
// Created by bantdit on 9/4/19.
//

#ifndef CYCLONITE_DEVICE_H
#define CYCLONITE_DEVICE_H

#include "handle.h"

namespace cyclonite::vulkan {
class Device
{
public:
    template<typename SurfaceType, template<typename> class Surface>
    explicit Device(VkPhysicalDevice const& vkPhysicalDevice, std::vector<Surface<SurfaceType>> const& surfaces);

    Device(Device const&) = delete;

    Device(Device&&) = default;

    ~Device() = default;

public:
    auto operator=(Device const&) -> Device& = delete;

    auto operator=(Device &&) -> Device& = default;

public:
    [[nodiscard]] auto handle() const -> VkDevice { return static_cast<VkDevice>(vkDevice_); }

private:
    static auto _extractPhysicalDeviceNameFromProperties(VkPhysicalDeviceProperties const& deviceProperties)
      -> std::string;

    static auto _findBestGraphicsQueueFamilyIndex(std::vector<VkQueueFamilyProperties> const& familyProperties)
      -> uint32_t;

    static auto _findBestComputeQueueFamilyIndex(std::vector<VkQueueFamilyProperties> const& familyProperties,
                                                 uint32_t graphicsQueueFamilyIndex) -> uint32_t;

    static auto _findBestHostTransferQueueFamilyIndex(std::vector<VkQueueFamilyProperties> const& familyProperties,
                                                      uint32_t graphicsQueueFamilyIndex) -> uint32_t;

private:
    VkPhysicalDevice vkPhysicalDevice_;
    VkPhysicalDeviceProperties vkDeviceProperties_;
    VkPhysicalDeviceMemoryProperties vkPhysicalDeviceMemoryProperties_;
    std::string name_;
    size_t graphicsQueueIndex_;
    size_t computeQueueIndex_;
    size_t deviceHostTransferIndex_;
    size_t presentationQueueIndex_;
    Handle<VkDevice> vkDevice_;
};

template<typename SurfaceType, template<typename> class Surface>
Device::Device(VkPhysicalDevice const& vkPhysicalDevice, std::vector<Surface<SurfaceType>> const& surfaces)
  : vkPhysicalDevice_{ vkPhysicalDevice }
  , vkDeviceProperties_{}
  , vkPhysicalDeviceMemoryProperties_{}
  , name_{ "" }
  , graphicsQueueIndex_{ std::numeric_limits<size_t>::max() }
  , computeQueueIndex_{ std::numeric_limits<size_t>::max() }
  , deviceHostTransferIndex_{ std::numeric_limits<size_t>::max() }
  , presentationQueueIndex_{ std::numeric_limits<size_t>::max() }
  , vkDevice_{ vkDestroyDevice }
{
    vkGetPhysicalDeviceProperties(vkPhysicalDevice_, &vkDeviceProperties_);
    vkGetPhysicalDeviceMemoryProperties(vkPhysicalDevice_, &vkPhysicalDeviceMemoryProperties_);

    name_ = _extractPhysicalDeviceNameFromProperties(vkDeviceProperties_);

    uint32_t familyCount = 0;

    vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice_, &familyCount, nullptr);

    std::vector<VkQueueFamilyProperties> familyPropertiesList(familyCount);

    vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice_, &familyCount, familyPropertiesList.data());

    uint32_t graphicsQueueFamilyIndex = _findBestGraphicsQueueFamilyIndex(familyPropertiesList);

    if (graphicsQueueFamilyIndex == std::numeric_limits<uint32_t>::max()) {
        throw std::runtime_error("could not found valid graphics queue family index for device: " + name_);
    }

    uint32_t computeQueueFamilyIndex = _findBestComputeQueueFamilyIndex(familyPropertiesList, graphicsQueueFamilyIndex);

    if (computeQueueFamilyIndex == std::numeric_limits<uint32_t>::max()) {
        throw std::runtime_error("could not found valid compute queue family index for device: " + name_);
    }

    uint32_t deviceHostTransferQueueFamilyIndex =
      _findBestHostTransferQueueFamilyIndex(familyPropertiesList, graphicsQueueFamilyIndex);

    VkPhysicalDeviceFeatures features = {};

    vkGetPhysicalDeviceFeatures(vkPhysicalDevice_, &features);

    // turn off unused features (for now)
    features.robustBufferAccess = VK_FALSE;
    features.shaderFloat64 = VK_FALSE;
    features.shaderInt64 = VK_FALSE;
    features.inheritedQueries = VK_FALSE;
}
}

#endif // CYCLONITE_DEVICE_H
