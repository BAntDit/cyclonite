//
// Created by bantdit on 11/3/19.
//

#ifndef CYCLONITE_FILLIMAGECREATIONINFO_H
#define CYCLONITE_FILLIMAGECREATIONINFO_H

#include <algorithm>
#include <array>
#include <cassert>
#include <variant>
#include <vulkan/vulkan.h>

namespace cyclonite::vulkan::internal {
static void fillImageCreationInfo(
  VkImageCreateInfo& imageCreateInfo,
  VkImageType imageType,
  VkImageCreateFlags imageCreateFlags,
  uint32_t width,
  uint32_t height,
  uint32_t depth,
  uint32_t countMipLevels,
  uint32_t arrayLayers,
  VkFormat format,
  VkImageTiling tiling,
  VkImageUsageFlags imageUsageFlags,
  VkSampleCountFlagBits sampleCount,
  VkImageLayout imageInitialLayout,
  std::variant<std::array<uint32_t, 1>, std::array<uint32_t, 2>, std::array<uint32_t, 3>>& reqFamilyIndices)
{
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.imageType = imageType;
    imageCreateInfo.flags = imageCreateFlags;
    imageCreateInfo.extent.width = width;
    imageCreateInfo.extent.height = height;
    imageCreateInfo.extent.depth = depth;
    imageCreateInfo.mipLevels = countMipLevels;
    imageCreateInfo.arrayLayers = arrayLayers;
    imageCreateInfo.format = format;
    imageCreateInfo.tiling = tiling;
    imageCreateInfo.initialLayout = imageInitialLayout;
    imageCreateInfo.usage = imageUsageFlags;
    imageCreateInfo.samples = sampleCount;

    std::visit(
      [&](auto& indices) -> void {
          std::sort(indices.begin(), indices.end());

          auto last = std::unique(indices.begin(), indices.end());

          auto length = std::distance(indices.begin(), last);

          assert(length > 0);

          imageCreateInfo.sharingMode = length > 1 ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
          imageCreateInfo.queueFamilyIndexCount = length;
          imageCreateInfo.pQueueFamilyIndices = indices.data();
      },
      reqFamilyIndices);
}
}

#endif // CYCLONITE_FILLIMAGECREATIONINFO_H
