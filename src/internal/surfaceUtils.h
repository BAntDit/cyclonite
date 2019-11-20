//
// Created by bantdit on 11/17/19.
//

#ifndef CYCLONITE_SURFACEUTILS_H
#define CYCLONITE_SURFACEUTILS_H

#include <array>
#include <vector>
#include <vulkan/vulkan.h>

namespace cyclonite::internal {
template<size_t formatCount>
static auto _chooseSurfaceFormat(VkPhysicalDevice physicalDevice,
                                 VkSurfaceKHR surface,
                                 std::array<std::pair<VkFormat, VkColorSpaceKHR>, formatCount> const& candidates)
  -> VkSurfaceFormatKHR
{
    uint32_t availableFormatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &availableFormatCount, nullptr);

    assert(availableFormatCount > 0);

    std::vector<VkSurfaceFormatKHR> availableFormats(availableFormatCount);

    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &availableFormatCount, availableFormats.data());

    for (auto&& [requiredFormat, requiredColorSpace] : candidates) {
        for (auto&& availableFormat : availableFormats) {
            if (requiredFormat == availableFormat.format && requiredColorSpace == availableFormat.colorSpace) {
                return availableFormat;
            }
        }
    }

    throw std::runtime_error("surface does not support required formats.");
}

template<size_t modeCount>
static auto _choosePresentationMode(VkPhysicalDevice physicalDevice,
                                    VkSurfaceKHR surface,
                                    std::array<VkPresentModeKHR, modeCount> const& candidates) -> VkPresentModeKHR
{
    uint32_t availableModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &availableModeCount, nullptr);

    assert(availableModeCount > 0);

    std::vector<VkPresentModeKHR> availablePresentModes(availableModeCount);

    vkGetPhysicalDeviceSurfacePresentModesKHR(
      physicalDevice, surface, &availableModeCount, availablePresentModes.data());

    for (auto&& candidate : candidates) {
        for (auto&& availableMode : availablePresentModes) {
            if (availableMode == candidate) {
                return availableMode;
            }
        }
    }

    throw std::runtime_error("surface does support any required presentation modes");
}
}

#endif // CYCLONITE_SURFACEUTILS_H
