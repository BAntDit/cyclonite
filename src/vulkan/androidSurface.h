//
// Created by bantdit on 9/19/19.
//

#ifndef CYCLONITE_ANDROIDSURFACE_H
#define CYCLONITE_ANDROIDSURFACE_H

#include "platform.h"

#if defined(VK_USE_PLATFORM_ANDROID_KHR)

#include "handle.h"

class AndroidSurface
{
public:
    AndroidSurface(VkInstance vkInstance, ANativeWindow* window);

    AndroidSurface(AndroidSurface const&) = delete;

    AndroidSurface(AndroidSurface&&) = default;

    auto operator=(AndroidSurface const&) -> AndroidSurface& = delete;

    auto operator=(AndroidSurface &&) -> AndroidSurface& = default;

    ~AndroidSurface() = default;

    [[nodiscard]] auto handle() const -> VkSurfaceKHR { return static_cast<VkSurfaceKHR>(vkSurfaceKHR_); }

private:
    Handle<VkSurfaceKHR> vkSurfaceKHR_;
};

#endif

#endif // CYCLONITE_ANDROIDSURFACE_H
