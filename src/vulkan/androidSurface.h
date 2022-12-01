//
// Created by bantdit on 9/19/19.
//

#ifndef CYCLONITE_ANDROIDSURFACE_H
#define CYCLONITE_ANDROIDSURFACE_H

#include "platform.h"

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
#include "baseSurface.h"
namespace cyclonite::vulkan {
class AndroidSurface : public BaseSurface
{
public:
    AndroidSurface(VkInstance vkInstance, ANativeWindow* window);

    AndroidSurface(AndroidSurface const&) = delete;

    AndroidSurface(AndroidSurface&&) = default;

    auto operator=(AndroidSurface const&) -> AndroidSurface& = delete;

    auto operator=(AndroidSurface &&) -> AndroidSurface& = default;

    ~AndroidSurface() = default;
};

using platform_surface_t = AndroidSurface;

using platform_surface_argument_type_list_t = easy_mp::type_list<ANativeWindow*>;
}
#endif

#endif // CYCLONITE_ANDROIDSURFACE_H
