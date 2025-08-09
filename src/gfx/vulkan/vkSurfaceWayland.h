//
// Created by anton on 8/9/25.
//

#ifndef CYCLONITE_GFX_VKSURFACEWAYLAND_H
#define CYCLONITE_GFX_VKSURFACEWAYLAND_H

#include "vkSurfaceKHR.h"
#include <metrix/type_list.h>

#if defined(GFX_DRIVER_VULKAN) && defined(VK_USE_PLATFORM_WAYLAND_KHR)

namespace cyclonite::gfx::vulkan {
class SurfaceWayland : public SurfaceKHR
{
public:
    SurfaceWayland(VkInstance vkInstance, wl_display* display, wl_surface* surface);

    SurfaceWayland(SurfaceWayland const&) = delete;

    SurfaceWayland(SurfaceWayland&&) = default;

    ~SurfaceWayland() = default;

    auto operator=(SurfaceWayland const&) -> SurfaceWayland& = delete;

    auto operator=(SurfaceWayland&&) -> SurfaceWayland& = default;
};

using platform_surface_t = SurfaceWayland;

using platform_surface_argument_type_list_t = metrix::type_list<wl_display*, wl_surface*>;
}

#endif // GFX_DRIVER_VULKAN && VK_USE_PLATFORM_XLIB_KHR
#endif // CYCLONITE_GFX_VKSURFACEWAYLAND_H
