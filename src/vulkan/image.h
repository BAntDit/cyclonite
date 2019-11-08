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

    Image(VkImage vkImage,
          uint32_t width,
          uint32_t height,
          VkFormat format = VK_FORMAT_R8G8B8A8_UINT,
          VkImageTiling tiling = VK_IMAGE_TILING_LINEAR);

    Image(Image const&) = delete;

    Image(Image&&) = default;

    ~Image() = default;

    auto operator=(Image const&) -> Image& = delete;

    auto operator=(Image &&) -> Image& = default;

    [[nodiscard]] auto handle() const -> VkImage { return static_cast<VkImage>(vkImage_); }

    [[nodiscard]] auto allocatedMemory() const -> MemoryPage::AllocatedMemory const& { return allocatedMemory_; }

    [[nodiscard]] auto width() const -> uint32_t { return width_; };

    [[nodiscard]] auto height() const -> uint32_t { return height_; };

    [[nodiscard]] auto depth() const -> uint32_t { return depth_; }

    [[nodiscard]] auto mipLevelCount() const -> uint32_t { return mipLevelCount_; }

    [[nodiscard]] auto arrayLayerCount() const -> uint32_t { return arrayLayerCount_; }

    [[nodiscard]] auto format() const -> VkFormat { return format_; }

    [[nodiscard]] auto imageType() const -> VkImageType { return imageType_; }

    [[nodiscard]] auto tiling() const -> VkImageTiling { return tiling_; }

private:
    uint32_t width_;
    uint32_t height_;
    uint32_t depth_;
    uint32_t mipLevelCount_;
    uint32_t arrayLayerCount_;

    VkFormat format_;

    VkImageType imageType_;

    VkImageTiling tiling_;

    MemoryPage::AllocatedMemory allocatedMemory_;
    Handle<VkImage> vkImage_;
};

using ImagePtr = std::shared_ptr<Image>;
}

#endif // CYCLONITE_IMAGE_H
