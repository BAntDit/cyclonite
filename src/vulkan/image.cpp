//
// Created by bantdit on 11/3/19.
//

#include "image.h"
#include "device.h"
#include "internal/fillImageCreationInfo.h"

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
             VkMemoryPropertyFlags memoryPropertieFlags)
  : allocatedMemory_{}
  , vkImage_{ device.handle(), vkDestroyImage }
{
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
                                    imageCreateFlags,
                                    sampleCount,
                                    imageInitialLayout,
                                    ownerQueueFamilyIndices);
}
}
