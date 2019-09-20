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
    bool _testExtensions(std::vector<const char*> const& requiredExtensions); // TODO:: ...

    void _defineQueueIndices(std::vector<uint32_t>& queueFamilyIndices,
                             uint32_t graphicsQueueFamilyIndex,
                             uint32_t computeQueueFamilyIndex,
                             uint32_t hostTransferQueueFamilyIndex,
                             uint32_t presentationQueueFamilyIndex);

    static void _defineQueuesInfo(std::vector<uint32_t> const& queueFamilyIndices,
                                  std::vector<VkDeviceQueueCreateInfo>& deviceQueuesCreateInfo);

    static auto _extractPhysicalDeviceNameFromProperties(VkPhysicalDeviceProperties const& deviceProperties)
      -> std::string;

    static auto _findBestGraphicsQueueFamilyIndex(std::vector<VkQueueFamilyProperties> const& familyProperties)
      -> uint32_t;

    static auto _findBestComputeQueueFamilyIndex(std::vector<VkQueueFamilyProperties> const& familyProperties,
                                                 uint32_t graphicsQueueFamilyIndex) -> uint32_t;

    static auto _findBestHostTransferQueueFamilyIndex(std::vector<VkQueueFamilyProperties> const& familyProperties,
                                                      uint32_t graphicsQueueFamilyIndex) -> uint32_t;

    template<typename SurfaceType, template<typename> class Surface>
    static auto _findBestPresentationQueueFamilyIndex(VkPhysicalDevice const& physicalDevice,
                                                      std::vector<VkQueueFamilyProperties> const& familyProperties,
                                                      uint32_t graphicsQueueFamilyIndex,
                                                      uint32_t deviceHostTransferQueueFamilyIndex,
                                                      std::vector<Surface<SurfaceType>> const& surfaces) -> uint32_t;

private:
    VkPhysicalDevice vkPhysicalDevice_;
    VkPhysicalDeviceProperties vkDeviceProperties_;
    VkPhysicalDeviceMemoryProperties vkPhysicalDeviceMemoryProperties_;
    std::string name_;
    size_t graphicsQueueIndex_;
    size_t computeQueueIndex_;
    size_t deviceHostTransferQueueIndex_;
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
  , deviceHostTransferQueueIndex_{ std::numeric_limits<size_t>::max() }
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

    bool presentationRequired = surfaces.size() > 0;

    uint32_t presentationQueueFamilyIndex = _findBestPresentationQueueFamilyIndex(
      vkPhysicalDevice_, familyPropertiesList, graphicsQueueFamilyIndex, deviceHostTransferQueueFamilyIndex, surfaces);

    if (presentationRequired && presentationQueueFamilyIndex == std::numeric_limits<uint32_t>::max()) {
        throw std::runtime_error("could not found valid presentation queue family index for device: " + name_);
    }

    std::vector<uint32_t> queueFamilyIndices = {};

    _defineQueueIndices(queueFamilyIndices,
                        graphicsQueueFamilyIndex,
                        computeQueueFamilyIndex,
                        deviceHostTransferQueueFamilyIndex,
                        presentationQueueFamilyIndex);

    std::vector<VkDeviceQueueCreateInfo> deviceQueuesCreateInfo = {};

    _defineQueuesInfo(queueFamilyIndices, deviceQueuesCreateInfo);

    std::vector<const char*> requiredExtensions = {};

    if (presentationRequired) {
        requiredExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    }

    VkPhysicalDeviceFeatures features = {};

    vkGetPhysicalDeviceFeatures(vkPhysicalDevice_, &features);

    // turn off unused features (for now)
    features.robustBufferAccess = VK_FALSE;
    features.shaderFloat64 = VK_FALSE;
    features.shaderInt64 = VK_FALSE;
    features.inheritedQueries = VK_FALSE;
}

template<typename SurfaceType, template<typename> class Surface>
auto Device::_findBestPresentationQueueFamilyIndex(VkPhysicalDevice const& physicalDevice,
                                                   std::vector<VkQueueFamilyProperties> const& familyProperties,
                                                   uint32_t graphicsQueueFamilyIndex,
                                                   uint32_t deviceHostTransferQueueFamilyIndex,
                                                   std::vector<Surface<SurfaceType>> const& surfaces) -> uint32_t
{
    uint32_t presentationQueueFamilyIndex = std::numeric_limits<uint32_t>::max();

    auto testQueueFamilyIndex = [&](uint32_t familyIndex) -> bool {
        bool presentationSupportResult = true;

        for (auto const& surface : surfaces) {
            VkBool32 presentationSupport = VK_FALSE;

            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, familyIndex, surface.handle(), &presentationSupport);

            if (presentationSupport == VK_FALSE) {
                presentationSupportResult = false;
                break;
            }
        }

        return presentationSupportResult;
    };

    if (surfaces.size() > 0 && testQueueFamilyIndex(graphicsQueueFamilyIndex)) {
        presentationQueueFamilyIndex = graphicsQueueFamilyIndex;
    } else if (surfaces.size() > 0) {
        for (size_t i = 0, count = familyProperties.size(); i < count; i++) {
            auto const& props = familyProperties[i];

            if (props.queueCount < 1 || i == graphicsQueueFamilyIndex || i == deviceHostTransferQueueFamilyIndex)
                continue;

            if (testQueueFamilyIndex(i)) {
                presentationQueueFamilyIndex = i;
                break;
            }
        }
    }

    return presentationQueueFamilyIndex;
}
}

#endif // CYCLONITE_DEVICE_H
