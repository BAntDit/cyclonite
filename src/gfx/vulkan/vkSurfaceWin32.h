//
// Created by anton on 8/9/25.
//

#ifndef CYCLONITE_GFX_VKSURFACEWIN32_H
#define CYCLONITE_GFX_VKSURFACEWIN32_H

#include "vkSurfaceKHR.h"
#include <metrix/type_list.h>

#if defined(GFX_DRIVER_VULKAN) && defined(VK_USE_PLATFORM_WIN32_KHR)

namespace cyclonite::gfx::vulkan {
class SurfaceWin32 : public SurfaceKHR
{
    SurfaceWin32(VkInstance vkInstance, HINSTANCE hinstance, HWND hwnd);

    SurfaceWin32(SurfaceWin32 const&) = delete;

    SurfaceWin32(SurfaceWin32&&) = default;

    ~SurfaceWin32() = default;

    auto operator=(SurfaceWin32 const&) -> SurfaceWin32& = delete;

    auto operator=(SurfaceWin32&&) -> SurfaceWin32& = default;
};

using platform_surface_t = SurfaceWin32;

using platform_surface_argument_type_list_t = metrix::type_list<HINSTANCE, HWND>;
}

#endif // GFX_DRIVER_VULKAN && VK_USE_PLATFORM_WIN32_KHR
#endif // CYCLONITE_GFX_VKSURFACEWIN32_H
