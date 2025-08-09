//
// Created by anton on 6/11/25.
//

#ifndef GFX_VK_SURFACE_H
#define GFX_VK_SURFACE_H

#include "core/resourceBase.h"
#include "gfx/common.h"

#if defined(GFX_DRIVER_VULKAN)

#include "vkSurfaceKHR.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_video.h>
#include <functional>
#include <memory>
#include <string_view>
#include <vulkan/vulkan.h>

namespace cyclonite::gfx::vulkan {
class Surface : public core::ResourceBase
{
public:
    Surface(core::ResourceManagerBase* resourceManager,
            core::ResourceId resourceId,
            core::ResourceRef deviceRef,
            uint32_t width,
            uint32_t height,
            std::string_view title,
            SurfaceFlagBits flags);

    ~Surface() = default;

    [[nodiscard]] auto width() const -> uint32_t { return extent_.width; }

    [[nodiscard]] auto height() const -> uint32_t { return extent_.height; }

private:
    VkExtent2D extent_;
    std::unique_ptr<SDL_Window, std::function<void(SDL_Window*)>> sdlWindowPtr_;
    std::unique_ptr<SurfaceKHR> surface_;
};
}

#endif // GFX_DRIVER_VULKAN
#endif // GFX_VK_SURFACE_H
