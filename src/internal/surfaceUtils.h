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
static auto _chooseSurfaceFormat(std::vector<VkSurfaceFormatKHR> const& availableFormats,
                                 std::array<std::pair<VkFormat, VkColorSpaceKHR>, formatCount> const& candidates)
  -> VkSurfaceFormatKHR
{
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
static auto _choosePresentationMode(std::vector<VkPresentModeKHR> const& availableModes,
                                    std::array<VkPresentModeKHR, modeCount> const& candidates) -> VkPresentModeKHR
{
    for (auto&& candidate : candidates) {
        for (auto&& availableMode : availableModes) {
            if (availableMode == candidate) {
                return availableMode;
            }
        }
    }

    throw std::runtime_error("surface does support any required presentation modes");
}
}

#endif // CYCLONITE_SURFACEUTILS_H
