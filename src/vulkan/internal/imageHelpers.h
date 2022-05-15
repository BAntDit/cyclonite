//
// Created by bantdit on 11/8/19.
//

#ifndef CYCLONITE_IMAGEHELPERS_H
#define CYCLONITE_IMAGEHELPERS_H

#include <cassert>
#include <vulkan/vulkan.h>

namespace cyclonite::vulkan::internal {
inline auto viewTypeToImageType(VkImageViewType imageViewType) -> VkImageType
{
    switch (imageViewType) {
        case VK_IMAGE_VIEW_TYPE_1D:
        case VK_IMAGE_VIEW_TYPE_1D_ARRAY: {
            return VK_IMAGE_TYPE_1D;
        }
        case VK_IMAGE_VIEW_TYPE_2D:
        case VK_IMAGE_VIEW_TYPE_2D_ARRAY:
        case VK_IMAGE_VIEW_TYPE_CUBE:
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

inline auto _getDepthStencilImageLayoutByFormat(VkFormat format) -> VkImageLayout
{
    auto layout = VkImageLayout{ VK_IMAGE_LAYOUT_UNDEFINED };

    switch (format) {
        case VK_FORMAT_D16_UNORM:
        case VK_FORMAT_X8_D24_UNORM_PACK32:
        case VK_FORMAT_D32_SFLOAT:
            // TODO:: add extension to separate depth and stencil
            layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
            break;
        case VK_FORMAT_S8_UINT:
            layout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
            break;
        case VK_FORMAT_D16_UNORM_S8_UINT:
        case VK_FORMAT_D24_UNORM_S8_UINT:
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            break;
        default:
            layout = VK_IMAGE_LAYOUT_UNDEFINED;
    }

    return layout;
}
}

#endif // CYCLONITE_IMAGEHELPERS_H
