//
// Created by anton on 6/11/25.
//

#ifndef GFX_VK_SURFACE_H
#define GFX_VK_SURFACE_H

#include <vulkan/vulkan.h>

namespace cyclonite::gfx::vulkan
{
class Surface
{
public:

    [[nodiscard]] auto width() const -> uint32_t { return extent_.width; }

    [[nodiscard]] auto height() const -> uint32_t { return extent_.height; }

private:
    VkExtent2D extent_;
};
}

#endif // GFX_VK_SURFACE_H
