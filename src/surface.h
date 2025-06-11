//
// Created by bantdit on 9/17/19.
//

#ifndef CYCLONITE_SURFACE_H
#define CYCLONITE_SURFACE_H

#include "sdl/sdlWindow.h"
#include "vulkan/device.h"
#include "vulkan/imageView.h"
#include "vulkan/platformSurface.h"
#include "windowProperties.h"
#include <array>

namespace cyclonite {
class Surface
{
public:
    struct Capabilities
    {
        uint32_t minImageCount;
        uint32_t maxImageCount;
        VkCompositeAlphaFlagsKHR supportedCompositeAlpha;
        VkImageUsageFlags supportedUsageFlags;
        VkSurfaceTransformFlagsKHR supportedTransforms;
        VkSurfaceTransformFlagBitsKHR currentTransform;
    };

public:
    Surface(vulkan::Device const& device, WindowProperties const& windowProperties);

    Surface(Surface const&) = delete;

    Surface(Surface&&) = default;

    ~Surface() = default;

    auto operator=(Surface const&) -> Surface& = delete;

    auto operator=(Surface&&) -> Surface& = default;

    [[nodiscard]] auto handle() const -> VkSurfaceKHR { return platformSurface_.handle(); }

    [[nodiscard]] auto width() const -> uint32_t { return extent_.width; }

    [[nodiscard]] auto height() const -> uint32_t { return extent_.height; }

    [[nodiscard]] auto capabilities() const -> Capabilities const& { return capabilities_; }

private:
    Capabilities capabilities_;
    VkExtent2D extent_;
    sdl::SDLWindow window_;
    vulkan::platform_surface_t platformSurface_;
};

template<typename... SurfaceArgs>
static auto _createSurface(VkInstance vkInstance, sdl::SDLWindow const& window, metrix::type_list<SurfaceArgs...>)
  -> vulkan::platform_surface_t
{
    return vulkan::platform_surface_t{ vkInstance, window.get<SurfaceArgs>()... };
}
}

#endif // CYCLONITE_SURFACE_H
