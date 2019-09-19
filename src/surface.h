//
// Created by bantdit on 9/17/19.
//

#ifndef CYCLONITE_SURFACE_H
#define CYCLONITE_SURFACE_H

#include "options.h"
#include "sdl/sdlSupport.h"
#include <easy-mp/type_list.h>
#include <vulkan/vulkan.h>

namespace cyclonite {
// Xlib, HWnd, Android or some other platform surfaces
template<typename xPlatformSurface>
class Surface
{
public:
    using surface_type_t = xPlatformSurface;

    template<typename... SurfaceArgs>
    Surface(VkInstance vkInstance,
            Options::WindowProperties const& windowProperties,
            easy_mp::type_list<SurfaceArgs...>);

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
                                   easy_mp::type_list<SurfaceArgs...>)
  : window_{ windowProperties.title,
             windowProperties.left,
             windowProperties.top,
             windowProperties.width,
             windowProperties.height,
             static_cast<uint32_t>(windowProperties.fullscreen
                                     ? SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN | SDL_WINDOW_BORDERLESS
                                     : SDL_WINDOW_SHOWN) }
  , xSurface_{ vkInstance, window_.get<SurfaceArgs>()... }
{}
}

#endif // CYCLONITE_SURFACE_H
