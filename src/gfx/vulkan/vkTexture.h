//
// Created by anton on 7/27/25.
//

#ifndef CYCLONITE_VKTEXTURE_H
#define CYCLONITE_VKTEXTURE_H

#include "gfx/common.h"
#include "handle.h"
#include "vmaUsage.h"

#if defined(GFX_DRIVER_VULKAN)
namespace cyclonite::gfx::vulkan {
class Texture
{
protected:
    Texture(VkImageCreateFlags imageCreateFlags,
            TextureType textureType,
            Format format,
            uint32_t width,
            uint32_t height,
            uint32_t depth,
            uint32_t mipCount,
            uint32_t arrayLayerCount,
            VkImageTiling tiling,
            VkImageUsageFlags usageFlags);

private:
    VmaAllocation allocation_;
    VkImage vkImage_;
};
}
#endif // GFX_DRIVER_VULKAN
#endif // CYCLONITE_VKTEXTURE_H
