//
// Created by bantdit on 11/3/19.
//

#include "image.h"
#include "device.h"
#include "internal/fillImageCreationInfo.h"
#include "memoryManager.h"

namespace cyclonite::vulkan {
Image::Image(Device& device,
             owner_queue_family_indices_t ownerQueueFamilyIndices,
             uint32_t width,
             uint32_t height,
             uint32_t depth,
             VkFormat format,
             uint32_t countMipLevels,
             uint32_t arrayLayerCount,
             VkImageTiling tiling,
             VkSampleCountFlagBits sampleCount,
             VkImageUsageFlags imageUsageFlags,
             VkImageType imageType,
             VkImageLayout imageInitialLayout,
             VkImageCreateFlags imageCreateFlags,
             VkMemoryPropertyFlags memoryPropertyFlags)
  : width_{ width }
  , height_{ height }
  , depth_{ depth }
  , mipLevelCount_{ countMipLevels }
  , arrayLayerCount_{ arrayLayerCount }
  , format_{ format }
  , imageType_{ imageType }
  , tiling_{ tiling }
  , imageCreateFlags_{ imageCreateFlags }
  , allocatedMemory_{}
  , vkImage_{ device.handle(), vkDestroyImage }
{
    // TODO:: validate parameters

    VkImageCreateInfo imageCreateInfo = {};

    internal::fillImageCreationInfo(imageCreateInfo,
                                    imageType,
                                    imageCreateFlags,
                                    width,
                                    height,
                                    depth,
                                    countMipLevels,
                                    arrayLayerCount,
                                    format,
                                    tiling,
                                    imageUsageFlags,
                                    sampleCount,
                                    imageInitialLayout,
                                    ownerQueueFamilyIndices);

    if (auto result = vkCreateImage(device.handle(), &imageCreateInfo, nullptr, &vkImage_); result != VK_SUCCESS) {
        if (result == VK_ERROR_OUT_OF_HOST_MEMORY)
            throw std::runtime_error("not enough RAM to create image");

        if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY)
            throw std::runtime_error("not enough GPU Memory to create image");

        assert(false);
    }

    {
        VkMemoryRequirements memoryRequirements = {};
        vkGetImageMemoryRequirements(device.handle(), static_cast<VkImage>(vkImage_), &memoryRequirements);

        allocatedMemory_ = device.memoryManager().alloc(memoryRequirements, memoryPropertyFlags);
    }
}

Image::Image(cyclonite::vulkan::Device& device,
             uint32_t width,
             uint32_t height,
             uint32_t depth,
             uint32_t countMipLevels,
             uint32_t arrayLayerCount,
             VkFormat format,
             VkImageTiling tiling,
             VkImageUsageFlags imageUsageFlags,
             VkImageType imageType)
  : Image{ device,
           std::array<uint32_t, 1>{ device.graphicsQueueFamilyIndex() },
           width,
           height,
           depth,
           format,
           countMipLevels,
           arrayLayerCount,
           tiling,
           VK_SAMPLE_COUNT_1_BIT,
           imageUsageFlags,
           imageType,
           VK_IMAGE_LAYOUT_UNDEFINED,
           0,
           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT }
{}

Image::Image(VkImage vkImage, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling)
  : width_{ width }
  , height_{ height }
  , depth_{ 1 }
  , mipLevelCount_{ 1 }
  , arrayLayerCount_{ 1 }
  , format_{ format }
  , imageType_{ VK_IMAGE_TYPE_2D }
  , tiling_{ tiling }
  , allocatedMemory_{}
  , vkImage_{}
{
    *vkImage_._replace() = vkImage;
}
}
