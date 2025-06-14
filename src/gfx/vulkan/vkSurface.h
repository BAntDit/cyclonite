//
// Created by anton on 6/11/25.
//

#ifndef GFX_VK_SURFACE_H
#define GFX_VK_SURFACE_H

#include "gfx/resource.h"

#if defined(GFX_DRIVER_VULKAN)

#include <SDL3/SDL.h>
#include <SDL3/SDL_video.h>
#include <functional>
#include <memory>
#include <vulkan/vulkan.h>

namespace cyclonite::gfx::vulkan {
class Surface : public ResourceBase<vulkan::Surface>
{
public:
    // Surface(vulkan::Device const& device, WindowProperties const& windowProperties);

    ~Surface() = default;

    [[nodiscard]] auto width() const -> uint32_t { return extent_.width; }

    [[nodiscard]] auto height() const -> uint32_t { return extent_.height; }

private:
    void reset() {}

    VkExtent2D extent_;
    std::unique_ptr<SDL_Window, std::function<void(SDL_Window*)>> sdlWindowPtr_;
};
}

#endif // GFX_DRIVER_VULKAN
#endif // GFX_VK_SURFACE_H
