//
// Created by bantdit on 9/19/19.
//

#ifndef CYCLONITE_WLSURFACE_H
#define CYCLONITE_WLSURFACE_H

#include "platform.h"

#if defined(VK_USE_PLATFORM_WAYLAND_KHR)

#include "handle.h"

namespace cyclonite::vulkan {
class WlSurface
{
public:
    WlSurface(VkInstance vkInstance, wl_display* display, wl_surface* surface);

    WlSurface(WlSurface const&) = delete;

    WlSurface(WlSurface&&) = default;

    auto operator=(WlSurface const&) -> WlSurface& = delete;

    auto operator=(WlSurface &&) -> WlSurface& = default;

    ~WlSurface() = default;

    [[nodiscard]] auto handle() const -> VkSurfaceKHR { return static_cast<VkSurfaceKHR>(vkSurfaceKHR_); }

private:
    Handle<VkSurfaceKHR> vkSurfaceKHR_;
};
}

#endif

#endif // CYCLONITE_WLSURFACE_H
