//
// Created by bantdit on 9/19/19.
//

#ifndef CYCLONITE_WIN32SURFACE_H
#define CYCLONITE_WIN32SURFACE_H

#include "platform.h"

#if defined(VK_USE_PLATFORM_WIN32_KHR)
#include "baseSurface.h"
#include <metrix/type_list.h>

namespace cyclonite::vulkan {
class Win32Surface : public BaseSurface
{
public:
    Win32Surface(VkInstance vkInstance, HINSTANCE hinstance, HWND hwnd);

    Win32Surface(Win32Surface const&) = delete;

    Win32Surface(Win32Surface&&) = default;

    auto operator=(Win32Surface const&) -> Win32Surface& = delete;

    auto operator=(Win32Surface&&) -> Win32Surface& = default;

    ~Win32Surface() = default;
};

using platform_surface_t = Win32Surface;

using platform_surface_argument_type_list_t = metrix::type_list<HINSTANCE, HWND>;
}

#endif

#endif // CYCLONITE_WIN32SURFACE_H
