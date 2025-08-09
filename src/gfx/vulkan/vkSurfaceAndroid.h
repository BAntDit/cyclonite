//
// Created by anton on 8/9/25.
//

#ifndef CYCLONITE_GFX_VKSURFACEANDROID_H
#define CYCLONITE_GFX_VKSURFACEANDROID_H

#include "vkSurfaceKHR.h"
#include <metrix/type_list.h>

#if defined(GFX_DRIVER_VULKAN) && defined(VK_USE_PLATFORM_ANDROID_KHR)

namespace cyclonite::gfx::vulkan
{
class SurfaceAndroid: public SurfaceKHR
{
public:
    SurfaceAndroid(VkInstance vkInstance, ANativeWindow* window);

    SurfaceAndroid(SurfaceAndroid const&) = delete;

    SurfaceAndroid(SurfaceAndroid&&) = default;

    ~SurfaceAndroid() = default;

    auto operator=(SurfaceAndroid const&) -> SurfaceAndroid& = delete;

    auto operator=(SurfaceAndroid&&) -> SurfaceAndroid& = default;
};

using platform_surface_t = SurfaceAndroid;

using platform_surface_argument_type_list_t = metrix::type_list<ANativeWindow*>;
}

#endif // GFX_DRIVER_VULKAN && VK_USE_PLATFORM_ANDROID_KHR
#endif //CYCLONITE_GFX_VKSURFACEANDROID_H
