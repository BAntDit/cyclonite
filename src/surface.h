//
// Created by bantdit on 9/17/19.
//

#ifndef CYCLONITE_SURFACE_H
#define CYCLONITE_SURFACE_H

#include "options.h"
#include "sdl/sdlSupport.h"
#include <vulkan/vulkan.h>

namespace cyclonite {
// Xlib, HWnd, Xcb or some other platform surfaces
template<typename xPlatformSurface>
class Surface
{
public:
    using surface_type_t = xPlatformSurface;

    template<typename... SurfaceArgs>
    Surface(VkInstance vkInstance, Options::WindowProperties const& windowProperties, SurfaceArgs&&... surfaceArgs);

    Surface(Surface const&) = delete;

    Surface(Surface&&) = default;

    ~Surface() = default;

    auto operator=(Surface const&) -> Surface& = delete;

    auto operator=(Surface &&) -> Surface& = default;

    [[nodiscard]] auto handle() const -> VkSurfaceKHR { return xSurface_.template handle(); }

private:
    sdl::SDLWindow window_;
    surface_type_t xSurface_;
};

template<typename xPlatformSurface>
template<typename... SurfaceArgs>
Surface<xPlatformSurface>::Surface(VkInstance vkInstance,
                                   const cyclonite::Options::WindowProperties& windowProperties,
                                   SurfaceArgs&&... surfaceArgs)
  : window_{ windowProperties.title,
             windowProperties.left,
             windowProperties.top,
             windowProperties.width,
             windowProperties.height,
             static_cast<uint32_t>(windowProperties.fullscreen
                                     ? SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN | SDL_WINDOW_BORDERLESS
                                     : SDL_WINDOW_SHOWN) }
  , xSurface_{ vkInstance, std::forward<SurfaceArgs>(surfaceArgs)... }
{}
}

#endif // CYCLONITE_SURFACE_H
