//
// Created by bantdit on 11/8/19.
//

#ifndef CYCLONITE_IMAGEHELPERS_H
#define CYCLONITE_IMAGEHELPERS_H

#include <cassert>
#include <vulkan/vulkan.h>

namespace cyclonite::vulkan::internal {
static auto viewTypeToImageType(VkImageViewType imageViewType) -> VkImageType
{
    switch (imageViewType) {
        case VK_IMAGE_VIEW_TYPE_1D: {
            return VK_IMAGE_TYPE_1D;
        }
        case VK_IMAGE_VIEW_TYPE_1D_ARRAY: {
            return VK_IMAGE_TYPE_1D;
        }
        case VK_IMAGE_VIEW_TYPE_2D: {
            return VK_IMAGE_TYPE_2D;
        }
        case VK_IMAGE_VIEW_TYPE_2D_ARRAY: {
            return VK_IMAGE_TYPE_2D;
        }
        case VK_IMAGE_VIEW_TYPE_CUBE: {
            return VK_IMAGE_TYPE_2D;
        }
        case VK_IMAGE_VIEW_TYPE_CUBE_ARRAY: {
            return VK_IMAGE_TYPE_2D;
        }
        case VK_IMAGE_VIEW_TYPE_3D: {
            return VK_IMAGE_TYPE_3D;
        }
        default:
            assert(false);
    }
}
}

#endif // CYCLONITE_IMAGEHELPERS_H
