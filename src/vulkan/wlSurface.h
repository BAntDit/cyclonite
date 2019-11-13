//
// Created by bantdit on 9/19/19.
//

#ifndef CYCLONITE_WLSURFACE_H
#define CYCLONITE_WLSURFACE_H

#include "platform.h"

#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
#include "baseSurface.h"

namespace cyclonite::vulkan {
class WlSurface : public BaseSurface
{
public:
    WlSurface(VkInstance vkInstance, wl_display* display, wl_surface* surface);

    WlSurface(WlSurface const&) = delete;

    WlSurface(WlSurface&&) = default;

    auto operator=(WlSurface const&) -> WlSurface& = delete;

    auto operator=(WlSurface &&) -> WlSurface& = default;

    ~WlSurface() = default;
};

using platform_surface_t = WlSurface;

using platform_surface_argument_type_list_t = easy_mp::type_list<wl_display*, wl_surface*>;
}
#endif

#endif // CYCLONITE_WLSURFACE_H
