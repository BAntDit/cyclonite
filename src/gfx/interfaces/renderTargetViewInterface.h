
#ifndef GFX_INTERFACES_RTV_H
#define GFX_INTERFACES_RTV_H

#include "core/resourceWeakRef.h"
#include <concepts>

namespace cyclonite::gfx::interfaces {
template<typename T>
concept RenderTargetViewConcept = requires(T t) {
    { t.texture() } -> std::same_as<core::ResourceWeakRef>;
};

template<RenderTargetViewConcept PlatformImplementation>
class RenderTargetViewInterface : private PlatformImplementation
{
public:
    friend class core::ResourceBase;

    using PlatformImplementation::PlatformImplementation;
    using PlatformImplementation::resourceBase;
    using PlatformImplementation::texture;
};
}

#endif // GFX_INTERFACES_RTV_H