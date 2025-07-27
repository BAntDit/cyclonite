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
public:
private:
    VmaAllocation allocation_;
    VkImage image_;
};
}
#endif // GFX_DRIVER_VULKAN
#endif // CYCLONITE_VKTEXTURE_H
