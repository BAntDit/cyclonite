//
// Created by bantdit on 9/17/19.
//

#ifndef CYCLONITE_SURFACE_H
#define CYCLONITE_SURFACE_H

#include "options.h"
#include "sdl/sdlSupport.h"
#include "vulkan/platformSurface.h"

namespace cyclonite {
class Surface
{
public:
    struct Capabilities {
        uint32_t minSwapChainImageCount = 0;
        uint32_t maxSwapChainImageCount = 0;
    };

public:
    Surface(VkInstance vkInstance, VkPhysicalDevice physicalDevice, Options::WindowProperties const& windowProperties);

    Surface(Surface const&) = delete;

    Surface(Surface&&) = default;

    ~Surface() = default;

    auto operator=(Surface const&) -> Surface& = delete;

    auto operator=(Surface &&) -> Surface& = default;

    [[nodiscard]] auto handle() const -> VkSurfaceKHR { return platformSurface_.handle(); }
    
    [[nodiscard]] auto capabilities() const -> Capabilities const& { return capabilities_; }

private:
    sdl::SDLWindow window_;
    vulkan::platform_surface_t platformSurface_;
    Capabilities capabilities_;
};

template<typename... SurfaceArgs>
static vulkan::platform_surface_t _createSurface(VkInstance vkInstance,
                                                 sdl::SDLWindow const& window,
                                                 easy_mp::type_list<SurfaceArgs...>)
{
    return vulkan::platform_surface_t{ vkInstance, window.get<SurfaceArgs>()... };
}
}

#endif // CYCLONITE_SURFACE_H
