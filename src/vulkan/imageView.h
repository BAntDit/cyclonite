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
    ImageView(Device const& device,
              ImagePtr const& image,
              VkImageViewType imageViewType = VK_IMAGE_VIEW_TYPE_2D,
              VkImageAspectFlags imageAspectFlags = VK_IMAGE_ASPECT_COLOR_BIT,
              uint32_t baseMipLevel = 0,
              uint32_t levelCount = 1,
              uint32_t baseArrayLayer = 0,
              uint32_t layerCount = 1);

private:
    ImagePtr imagePtr_;
    Handle<VkImageView> vkImageView_;
};
}

#endif // CYCLONITE_IMAGEVIEW_H
