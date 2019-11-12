//
// Created by bantdit on 11/10/19.
//

#include "imageView.h"
#include "device.h"
#include "internal/imageHelpers.h"

namespace cyclonite::vulkan {
ImageView::ImageView(Device& device,
                     ImagePtr const& image,
                     VkImageViewType imageViewType,
                     VkImageAspectFlags imageAspectFlags,
                     uint32_t baseMipLevel,
                     uint32_t levelCount,
                     uint32_t baseArrayLayer,
                     uint32_t layerCount)
  : imagePtr_{ image }
  , vkImageView_{ device.handle(), vkDestroyImageView }
  , vkImageViewType_{ imageViewType }
{
    assert(image->type() == internal::viewTypeToImageType(imageViewType));

    assert(imageViewType != VK_IMAGE_VIEW_TYPE_1D ||
           (image->width() >= 1 && image->height() == 1 && image->depth() == 1 && layerCount == 1));

    assert(imageViewType != VK_IMAGE_VIEW_TYPE_1D_ARRAY ||
           (image->width() >= 1 && image->height() == 1 && image->depth() == 1 && layerCount >= 1));

    assert(imageViewType != VK_IMAGE_VIEW_TYPE_2D ||
           (image->width() >= 1 && image->height() >= 1 && image->depth() == 1 && layerCount == 1));

    assert(imageViewType != VK_IMAGE_VIEW_TYPE_2D_ARRAY ||
           (image->width() >= 1 && image->height() >= 1 && image->depth() == 1 && layerCount >= 1));

    assert(imageViewType != VK_IMAGE_VIEW_TYPE_CUBE ||
           (image->width() >= 1 && image->height() == image->width() && image->depth() == 1 && layerCount == 6 &&
            (image->createFlags() & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) != 0));

    assert(imageViewType != VK_IMAGE_VIEW_TYPE_CUBE_ARRAY ||
           (image->width() >= 1 && image->height() == image->width() && image->depth() == 1 && layerCount % 6 == 0 &&
            (image->createFlags() & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) != 0));

    assert(imageViewType != VK_IMAGE_VIEW_TYPE_3D ||
           (image->width() >= 1 && image->height() >= 1 && image->depth() >= 1 && layerCount == 1));

    VkImageViewCreateInfo imageViewCreateInfo = {};

    imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCreateInfo.image = image->handle();
    imageViewCreateInfo.viewType = imageViewType;
    imageViewCreateInfo.format = image->format();
    imageViewCreateInfo.subresourceRange.aspectMask = imageAspectFlags;
    imageViewCreateInfo.subresourceRange.baseMipLevel = baseMipLevel;
    imageViewCreateInfo.subresourceRange.levelCount = levelCount;
    imageViewCreateInfo.subresourceRange.baseArrayLayer = baseArrayLayer;
    imageViewCreateInfo.subresourceRange.layerCount = layerCount;

    if (auto result = vkCreateImageView(device.handle(), &imageViewCreateInfo, nullptr, &vkImageView_);
        result != VK_SUCCESS) {
        if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
            throw std::runtime_error("not enough device memory to create image view");
        }

        if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
            throw std::runtime_error("not enough host memory to create image view");
        }

        assert(false);
    }
}

ImageView::ImageView(Device& device,
                     cyclonite::vulkan::ImageView::owner_queue_family_indices_t ownerQueueFamilyIndices,
                     uint32_t width,
                     uint32_t height,
                     uint32_t depth,
                     VkFormat format,
                     uint32_t countMipLevels,
                     uint32_t arrayLayerCount,
                     VkImageViewType imageViewType,
                     VkImageAspectFlags imageAspectFlags,
                     VkImageTiling tiling,
                     VkSampleCountFlagBits sampleCount,
                     VkImageUsageFlags imageUsageFlags,
                     VkImageCreateFlags imageCreateFlags,
                     VkMemoryPropertyFlags memoryPropertiesFlags)
  : imagePtr_{ std::make_shared<Image>(device,
                                       ownerQueueFamilyIndices,
                                       width,
                                       height,
                                       depth,
                                       format,
                                       countMipLevels,
                                       arrayLayerCount,
                                       tiling,
                                       sampleCount,
                                       imageUsageFlags,
                                       internal::viewTypeToImageType(imageViewType),
                                       VK_IMAGE_LAYOUT_UNDEFINED,
                                       imageCreateFlags,
                                       memoryPropertiesFlags) }
  , vkImageView_{ device.handle(), vkDestroyImageView }
  , vkImageViewType_{ imageViewType }
{
    assert(imageViewType != VK_IMAGE_VIEW_TYPE_1D || (width >= 1 && height == 1 && depth == 1 && arrayLayerCount == 1));

    assert(imageViewType != VK_IMAGE_VIEW_TYPE_1D_ARRAY ||
           (width >= 1 && height == 1 && depth == 1 && arrayLayerCount >= 1));

    assert(imageViewType != VK_IMAGE_VIEW_TYPE_2D || (width >= 1 && height >= 1 && depth == 1 && arrayLayerCount == 1));

    assert(imageViewType != VK_IMAGE_VIEW_TYPE_2D_ARRAY ||
           (width >= 1 && height >= 1 && depth == 1 && arrayLayerCount >= 1));

    assert(imageViewType != VK_IMAGE_VIEW_TYPE_CUBE ||
           (width >= 1 && width == height && depth == 1 && arrayLayerCount == 6 &&
            (imageCreateFlags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) != 0));

    assert(imageViewType != VK_IMAGE_VIEW_TYPE_CUBE_ARRAY ||
           (width >= 1 && width == height && depth == 1 && arrayLayerCount % 6 == 0 &&
            (imageCreateFlags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) != 0));

    assert(imageViewType != VK_IMAGE_VIEW_TYPE_3D || (width >= 1 && height >= 1 && depth >= 1 && arrayLayerCount == 1));

    VkImageViewCreateInfo imageViewCreateInfo = {};

    imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCreateInfo.image = imagePtr_->handle();
    imageViewCreateInfo.viewType = imageViewType;
    imageViewCreateInfo.format = format;
    imageViewCreateInfo.subresourceRange.aspectMask = imageAspectFlags;
    imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    imageViewCreateInfo.subresourceRange.levelCount = countMipLevels;
    imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    imageViewCreateInfo.subresourceRange.layerCount = arrayLayerCount;

    if (auto result = vkCreateImageView(device.handle(), &imageViewCreateInfo, nullptr, &vkImageView_);
        result != VK_SUCCESS) {
        if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
            throw std::runtime_error("not enough device memory to create image view");
        }

        if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
            throw std::runtime_error("not enough host memory to create image view");
        }

        assert(false);
    }
}
}
