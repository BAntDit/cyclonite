//
// Created by bantdit on 9/19/19.
//

#ifndef CYCLONITE_WIN32SURFACE_H
#define CYCLONITE_WIN32SURFACE_H

#include "handle.h"
#include "platform.h"

namespace cyclonite::vulkan {
class Win32Surface
{
public:
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    Win32Surface(VkInstance vkInstance, HINSTANCE hinstance, HWND hwnd);

    Win32Surface(Win32Surface const&) = delete;

    Win32Surface(Win32Surface&&) = default;

    auto operator=(Win32Surface const&) -> Win32Surface& = delete;

    auto operator=(Win32Surface &&) -> Win32Surface& = default;

    ~Win32Surface() = default;

    [[nodiscard]] auto handle() const -> VkSurfaceKHR { return static_cast<VkSurfaceKHR>(vkSurfaceKHR_); }
#else
    [[nodiscard]] auto handle() const -> VkSurfaceKHR { return VK_NULL_HANDLE; }
#endif
private:
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    Handle<VkSurfaceKHR> vkSurfaceKHR_;
#endif
};
}

#endif // CYCLONITE_WIN32SURFACE_H
