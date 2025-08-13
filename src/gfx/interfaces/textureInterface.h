
#ifndef GFX_INTERFACES_TEXTURE_H
#define GFX_INTERFACES_TEXTURE_H
#include "core/resourceRef.h"
#include "gfx/common.h"
#include <concepts>
#include <cstdint>

namespace cyclonite::gfx::interfaces {
template<typename T>
concept TextureConcept = requires(T t)
{
    {
        t.currentState()
    }
    ->std::same_as<TextureState>;

    {
        t.format()
    }
    ->std::same_as<Format>;
};

template<TextureConcept PlatformImplementation>
class TextureInterface : private PlatformImplementation
{
public:
    friend class core::ResourceBase;

    using PlatformImplementation::currentState;
    using PlatformImplementation::format;
    using PlatformImplementation::PlatformImplementation;
};
}

#endif // GFX_INTERFACES_TEXTURE_H