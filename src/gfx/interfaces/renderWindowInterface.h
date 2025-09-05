//
// Created by anton on 6/11/25.
//

#ifndef GFX_INTERFACES_RENDER_WINDOW_H
#define GFX_INTERFACES_RENDER_WINDOW_H

#include "core/resourceRef.h"
#include "gfx/common.h"
#include <concepts>
#include <cstdint>

namespace cyclonite::gfx::interfaces {
template<typename T>
concept RenderWindowConcept = requires(T t)
{
    {
        t.width()
    }
    ->std::same_as<uint32_t>;
    {
        t.height()
    }
    ->std::same_as<uint32_t>;

    {
        t.swapchainLength()
    }
    ->std::same_as<uint32_t>;

    {
        t.hasDepth()
    }
    ->std::same_as<bool>;

    {
        t.colorOutputFormat()
    }
    ->std::same_as<gfx::Format>;

    {
        t.depthStencilFormat()
    }
    ->std::same_as<gfx::Format>;

    {
        t.presentMode()
    }
    ->std::same_as<gfx::PresentMode>;
};

template<RenderWindowConcept PlatformImplementation>
class RenderWindowInterface : private PlatformImplementation
{
public:
    friend class core::ResourceBase;

    using PlatformImplementation::height;
    using PlatformImplementation::PlatformImplementation;
    using PlatformImplementation::resourceBase;
    using PlatformImplementation::swapchainLength;
    using PlatformImplementation::width;
    using PlatformImplementation::hasDepth;
    using PlatformImplementation::colorOutputFormat;
    using PlatformImplementation::depthStencilFormat;
    using PlatformImplementation::presentMode;
};
}

#endif // GFX_INTERFACES_RENDER_WINDOW_H
