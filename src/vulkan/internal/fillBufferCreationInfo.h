//
// Created by bantdit on 10/6/19.
//

#ifndef CYCLONITE_FILLBUFFERCREATIONINFO_H
#define CYCLONITE_FILLBUFFERCREATIONINFO_H

#include <algorithm>
#include <array>
#include <cassert>
#include <variant>
#include <vulkan/vulkan.h>

namespace cyclonite::vulkan::internal {
static void fillBufferCreationInfo(
  VkBufferCreateInfo& bufferCreateInfo,
  VkBufferUsageFlags usageFlags,
  VkDeviceSize size,
  std::variant<std::array<uint32_t, 1>, std::array<uint32_t, 2>, std::array<uint32_t, 3>>& reqFamilyIndices)
{
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.usage = usageFlags;
    bufferCreateInfo.size = size;

    std::visit(
      [&](auto& indices) -> void {
          std::sort(indices.begin(), indices.end());

          auto last = std::unique(indices.begin(), indices.end());

          auto length = std::distance(indices.begin(), last);

          assert(length > 0);

          bufferCreateInfo.sharingMode = length > 1 ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
          bufferCreateInfo.queueFamilyIndexCount = length;
          bufferCreateInfo.pQueueFamilyIndices = indices.data();
      },
      reqFamilyIndices);
}
}

#endif // CYCLONITE_FILLBUFFERCREATIONINFO_H
