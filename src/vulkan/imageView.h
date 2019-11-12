//
// Created by bantdit on 11/7/19.
//

#ifndef CYCLONITE_IMAGEVIEW_H
#define CYCLONITE_IMAGEVIEW_H

#include "image.h"
#include <memory>
#include <vulkan/vulkan.h>

namespace cyclonite::vulkan {
class ImageView
{
public:
    using owner_queue_family_indices_t =
      std::variant<std::array<uint32_t, 1>, std::array<uint32_t, 2>, std::array<uint32_t, 3>>;

    ImageView(Device& device,
              ImagePtr const& image,
              VkImageViewType imageViewType = VK_IMAGE_VIEW_TYPE_2D,
              VkImageAspectFlags imageAspectFlags = VK_IMAGE_ASPECT_COLOR_BIT,
              uint32_t baseMipLevel = 0,
              uint32_t levelCount = 1,
              uint32_t baseArrayLayer = 0,
              uint32_t layerCount = 1);

    ImageView(Device& device,
              owner_queue_family_indices_t ownerQueueFamilyIndices,
              uint32_t width,
              uint32_t height,
              uint32_t depth = 1,
              VkFormat format = VK_FORMAT_R8G8B8A8_UNORM,
              uint32_t countMipLevels = 1,
              uint32_t arrayLayerCount = 1,
              VkImageViewType imageViewType = VK_IMAGE_VIEW_TYPE_2D,
              VkImageAspectFlags imageAspectFlags = VK_IMAGE_ASPECT_COLOR_BIT,
              VkImageTiling tiling = VK_IMAGE_TILING_LINEAR,
              VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT,
              VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
              VkImageCreateFlags imageCreateFlags = 0,
              VkMemoryPropertyFlags memoryPropertiesFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    [[nodiscard]] auto handle() const -> VkImageView { return static_cast<VkImageView>(vkImageView_); }

    [[nodiscard]] auto type() const -> VkImageViewType { return vkImageViewType_; }

private:
    ImagePtr imagePtr_;
    Handle<VkImageView> vkImageView_;
    VkImageViewType vkImageViewType_;
};
}

#endif // CYCLONITE_IMAGEVIEW_H
