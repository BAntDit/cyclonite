//
// Created by anton on 6/11/25.
//

#ifndef GFX_INTERFACES_RENDER_WINDOW_H
#define GFX_INTERFACES_RENDER_WINDOW_H

#include "core/resourceSharedRef.h"
#include "gfx/common.h"
#include <concepts>
#include <cstdint>

namespace cyclonite::gfx::interfaces {
template<typename T>
concept RenderWindowConcept = requires(T t, uint64_t i) {
    { t.width() } -> std::same_as<uint32_t>;
    { t.height() } -> std::same_as<uint32_t>;

    { t.swapchainLength() } -> std::same_as<uint32_t>;

    { t.hasDepth() } -> std::same_as<bool>;

    { t.colorOutputFormat() } -> std::same_as<gfx::Format>;

    { t.depthStencilFormat() } -> std::same_as<gfx::Format>;

    { t.presentMode() } -> std::same_as<gfx::PresentMode>;

    { t.nextSwapchainIndex(i) } -> std::same_as<std::pair<uint32_t, core::ResourceSharedRef>>;

    { t.present() } -> std::same_as<void>;

    { t.presentationSignal() } -> std::same_as<core::ResourceSharedRef>;
};

template<RenderWindowConcept PlatformImplementation>
class RenderWindowInterface : private PlatformImplementation
{
public:
    friend class core::ResourceBase;

    using PlatformImplementation::colorOutputFormat;
    using PlatformImplementation::depthStencilFormat;
    using PlatformImplementation::hasDepth;
    using PlatformImplementation::height;
    using PlatformImplementation::nextSwapchainIndex;
    using PlatformImplementation::PlatformImplementation;
    using PlatformImplementation::present;
    using PlatformImplementation::presentationSignal;
    using PlatformImplementation::presentMode;
    using PlatformImplementation::resourceBase;
    using PlatformImplementation::swapchainLength;
    using PlatformImplementation::width;
};
}

#endif // GFX_INTERFACES_RENDER_WINDOW_H
