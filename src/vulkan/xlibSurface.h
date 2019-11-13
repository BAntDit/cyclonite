//
// Created by bantdit on 9/17/19.
//

#ifndef CYCLONITE_XLIBSURFACE_H
#define CYCLONITE_XLIBSURFACE_H

#include "platform.h"

#if defined(VK_USE_PLATFORM_XLIB_KHR)
#include "baseSurface.h"

namespace cyclonite::vulkan {
class XlibSurface : public BaseSurface
{
public:
    XlibSurface(VkInstance vkInstance, Display* display, Window const& window);

    XlibSurface(XlibSurface const&) = delete;

    XlibSurface(XlibSurface&&) = default;

    auto operator=(XlibSurface const&) -> XlibSurface& = delete;

    auto operator=(XlibSurface &&) -> XlibSurface& = default;

    ~XlibSurface() = default;
};
}
#endif

#endif // CYCLONITE_XLIBSURFACE_H
