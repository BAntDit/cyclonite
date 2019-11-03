//
// Created by bantdit on 11/3/19.
//

#ifndef CYCLONITE_IMAGE_H
#define CYCLONITE_IMAGE_H

#include "handle.h"
#include "memoryPage.h"
#include <variant>

namespace cyclonite::vulkan {
class Image
{
public:
    using owner_queue_family_indices_t =
      std::variant<std::array<uint32_t, 1>, std::array<uint32_t, 2>, std::array<uint32_t, 3>>;

    Image(Device& device,
          owner_queue_family_indices_t ownerQueueFamilyIndices,
          uint32_t width,
          uint32_t height,
          uint32_t depth = 1,
          VkFormat format = VK_FORMAT_R8G8B8A8_UINT,
          uint32_t countMipLevels = 1,
          uint32_t arrayLayerCount = 1,
          VkImageTiling tiling = VK_IMAGE_TILING_LINEAR,
          VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT,
          VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
          VkImageType imageType = VK_IMAGE_TYPE_2D,
          VkImageLayout imageInitialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
          VkImageCreateFlags imageCreateFlags = 0,
          VkMemoryPropertyFlags memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    Image(Device& device,
          uint32_t width,
          uint32_t height,
          uint32_t depth,
          uint32_t countMipLevels = 1,
          uint32_t arrayLayerCount = 1,
          VkFormat format = VK_FORMAT_R8G8B8A8_UINT,
          VkImageTiling tiling = VK_IMAGE_TILING_LINEAR,
          VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
          VkImageType imageType = VK_IMAGE_TYPE_2D);

    [[nodiscard]] auto handle() const -> VkImage { return static_cast<VkImage>(vkImage_); }

private:
    MemoryPage::AllocatedMemory allocatedMemory_;
    Handle<VkImage> vkImage_;
};
}

#endif // CYCLONITE_IMAGE_H
