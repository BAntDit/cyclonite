//
// Created by bantdit on 9/17/19.
//

#ifndef CYCLONITE_XLIBSURFACE_H
#define CYCLONITE_XLIBSURFACE_H

#include "handle.h"

namespace cyclonite::vulkan {
class XlibSurface
{
public:
    XlibSurface(VkInstance vkInstance, Display* display, Window const& window);

    XlibSurface(XlibSurface const&) = delete;

    XlibSurface(XlibSurface&&) = default;

    auto operator=(XlibSurface const&) -> XlibSurface& = delete;

    auto operator=(XlibSurface &&) -> XlibSurface& = default;

    ~XlibSurface() = default;

    [[nodiscard]] auto handle() const -> VkSurfaceKHR { return static_cast<VkSurfaceKHR>(vkSurfaceKHR_); }

private:
    Handle<VkSurfaceKHR> vkSurfaceKHR_;
};
}

#endif // CYCLONITE_XLIBSURFACE_H
