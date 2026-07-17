//
// Created by anton on 7/24/25.
//

#ifndef CYCLONITE_VKSURFACEKHR_H
#define CYCLONITE_VKSURFACEKHR_H

#include "handle.h"

#if defined(GFX_DRIVER_VULKAN)

namespace cyclonite::gfx::vulkan {
class SurfaceKHR
{
public:
    explicit SurfaceKHR(VkInstance vkInstance);

    SurfaceKHR(SurfaceKHR const&) = delete;

    SurfaceKHR(SurfaceKHR&&) = default;

    virtual ~SurfaceKHR() = default;

    auto operator=(SurfaceKHR const&) -> SurfaceKHR& = delete;

    auto operator=(SurfaceKHR&&) -> SurfaceKHR& = default;

    [[nodiscard]] auto handle() const -> VkSurfaceKHR { return static_cast<VkSurfaceKHR>(vkSurfaceKHR_); }

protected:
    Handle<VkSurfaceKHR> vkSurfaceKHR_;
};
}

#endif // GFX_DRIVER_VULKAN
#endif // CYCLONITE_VKSURFACEKHR_H
