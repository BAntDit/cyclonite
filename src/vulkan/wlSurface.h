//
// Created by bantdit on 9/19/19.
//

#ifndef CYCLONITE_WLSURFACE_H
#define CYCLONITE_WLSURFACE_H

#include "handle.h"
#include "platform.h"

namespace cyclonite::vulkan {
class WlSurface
{
public:
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
    WlSurface(VkInstance vkInstance, wl_display* display, wl_surface* surface);

    WlSurface(WlSurface const&) = delete;

    WlSurface(WlSurface&&) = default;

    auto operator=(WlSurface const&) -> WlSurface& = delete;

    auto operator=(WlSurface &&) -> WlSurface& = default;

    ~WlSurface() = default;

    [[nodiscard]] auto handle() const -> VkSurfaceKHR { return static_cast<VkSurfaceKHR>(vkSurfaceKHR_); }
#else
    [[nodiscard]] auto handle() const -> VkSurfaceKHR { return VK_NULL_HANDLE; }
#endif
private:
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
    Handle<VkSurfaceKHR> vkSurfaceKHR_;
#endif
};
}

#endif // CYCLONITE_WLSURFACE_H
