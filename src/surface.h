//
// Created by bantdit on 9/17/19.
//

#ifndef CYCLONITE_SURFACE_H
#define CYCLONITE_SURFACE_H

#include "options.h"
#include "sdl/sdlSupport.h"
#include "vulkan/device.h"
#include "vulkan/platformSurface.h"
#include <array>

namespace cyclonite {
class Surface
{
public:
    Surface(VkInstance vkInstance, vulkan::Device const& device, Options::WindowProperties const& windowProperties);

    Surface(Surface const&) = delete;

    Surface(Surface&&) = default;

    ~Surface() = default;

    auto operator=(Surface const&) -> Surface& = delete;

    auto operator=(Surface &&) -> Surface& = default;

    [[nodiscard]] auto handle() const -> VkSurfaceKHR { return platformSurface_.handle(); }

private:
    sdl::SDLWindow window_;
    vulkan::platform_surface_t platformSurface_;
    vulkan::Handle<VkSwapchainKHR> vkSwapchain_;
};

template<typename... SurfaceArgs>
static auto _createSurface(VkInstance vkInstance, sdl::SDLWindow const& window, easy_mp::type_list<SurfaceArgs...>)
  -> vulkan::platform_surface_t
{
    return vulkan::platform_surface_t{ vkInstance, window.get<SurfaceArgs>()... };
}
}

#endif // CYCLONITE_SURFACE_H
