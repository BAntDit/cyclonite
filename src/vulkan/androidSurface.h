//
// Created by bantdit on 9/19/19.
//

#ifndef CYCLONITE_ANDROIDSURFACE_H
#define CYCLONITE_ANDROIDSURFACE_H

#include "platform.h"
#include "handle.h"

class AndroidSurface
{
public:
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    AndroidSurface(VkInstance vkInstance, ANativeWindow* window);

    AndroidSurface(AndroidSurface const&) = delete;

    AndroidSurface(AndroidSurface&&) = default;

    auto operator=(AndroidSurface const&) -> AndroidSurface& = delete;

    auto operator=(AndroidSurface &&) -> AndroidSurface& = default;

    ~AndroidSurface() = default;

    [[nodiscard]] auto handle() const -> VkSurfaceKHR { return static_cast<VkSurfaceKHR>(vkSurfaceKHR_); }
#else
    [[nodiscard]] auto handle() const -> VkSurfaceKHR { return VK_NULL_HANDLE; }
#endif
private:
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    Handle<VkSurfaceKHR> vkSurfaceKHR_;
#endif
};

#endif

#endif // CYCLONITE_ANDROIDSURFACE_H
