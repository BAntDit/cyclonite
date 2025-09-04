//
// Created by anton on 6/11/25.
//

#ifndef GFX_INTERFACES_RENDER_WINDOW_H
#define GFX_INTERFACES_RENDER_WINDOW_H

#include "core/resourceRef.h"
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
};

template<RenderWindowConcept PlatformImplementation>
class RenderWindowInterface : private PlatformImplementation
{
public:
    friend class core::ResourceBase;

    using PlatformImplementation::height;
    using PlatformImplementation::PlatformImplementation;
    using PlatformImplementation::resourceBase;
    using PlatformImplementation::width;
    using PlatformImplementation::swapchainLength;
};
}

#endif // GFX_INTERFACES_RENDER_WINDOW_H
