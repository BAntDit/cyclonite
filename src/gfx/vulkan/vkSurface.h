//
// Created by anton on 6/11/25.
//

#ifndef GFX_VK_SURFACE_H
#define GFX_VK_SURFACE_H

#include "core/resourceBase.h"

#if defined(GFX_DRIVER_VULKAN)

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
    // TODO:: 
    Surface(core::ResourceManagerBase* resourceManager,
            core::ResourceId resourceId,
            VkInstance vkInstance,
            VkDevice vkDevice,
            uint32_t width,
            uint32_t height,
            bool fullscreen,
            std::string_view title);

    ~Surface() = default;

    [[nodiscard]] auto width() const -> uint32_t { return extent_.width; }

    [[nodiscard]] auto height() const -> uint32_t { return extent_.height; }

private:
    VkExtent2D extent_;
    std::unique_ptr<SDL_Window, std::function<void(SDL_Window*)>> sdlWindowPtr_;
};
}

#endif // GFX_DRIVER_VULKAN
#endif // GFX_VK_SURFACE_H
