//
// Created by anton on 8/9/25.
//

#ifndef CYCLONITE_GFX_VKSURFACEXLIB_H
#define CYCLONITE_GFX_VKSURFACEXLIB_H

#include "vkSurfaceKHR.h"
#include <metrix/type_list.h>

#if defined(GFX_DRIVER_VULKAN) && defined(VK_USE_PLATFORM_XLIB_KHR)

namespace cyclonite::gfx::vulkan {
class SurfaceXlib : public SurfaceKHR
{
public:
    SurfaceXlib(VkInstance vkInstance, Display* display, Window window);

    SurfaceXlib(SurfaceXlib const&) = delete;

    SurfaceXlib(SurfaceXlib&&) = default;

    ~SurfaceXlib() = default;

    auto operator=(SurfaceXlib const&) -> SurfaceXlib& = delete;

    auto operator=(SurfaceXlib&&) -> SurfaceXlib& = delete;
};

using platform_surface_t = SurfaceXlib;

using platform_surface_argument_type_list_t = metrix::type_list<Display*, Window>;
}

#endif // GFX_DRIVER_VULKAN && VK_USE_PLATFORM_XLIB_KHR
#endif // CYCLONITE_GFX_VKSURFACEXLIB_H
