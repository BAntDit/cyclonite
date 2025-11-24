
#ifndef GFX_INTERFACES_TEXTURE_H
#define GFX_INTERFACES_TEXTURE_H

#include "core/resourceWeakRef.h"
#include "gfx/common.h"
#include <concepts>
#include <cstdint>

namespace cyclonite::gfx::interfaces {
template<typename T>
concept TextureConcept = requires(T t, uint16_t a) {
    { t.currentState() } -> std::same_as<TextureState>;

    { t.format() } -> std::same_as<Format>;

    { t.width() } -> std::same_as<uint32_t>;

    { t.height() } -> std::same_as<uint32_t>;

    { t.depth() } -> std::same_as<uint32_t>;

    { t.mipCount() } -> std::same_as<uint32_t>;

    { t.type() } -> std::same_as<TextureType>;

    { t.sampler() } -> std::same_as<core::ResourceSharedRef>;

    { t.getRTV(a) } -> std::same_as<core::ResourceWeakRef>;

    { t.getSRV() } -> std::same_as<core::ResourceSharedRef>;
};

template<TextureConcept PlatformImplementation>
class TextureInterface : private PlatformImplementation
{
public:
    friend class core::ResourceBase;

    using PlatformImplementation::currentState;
    using PlatformImplementation::depth;
    using PlatformImplementation::format;
    using PlatformImplementation::getRTV;
    using PlatformImplementation::getSRV;
    using PlatformImplementation::height;
    using PlatformImplementation::mipCount;
    using PlatformImplementation::PlatformImplementation;
    using PlatformImplementation::resourceBase;
    using PlatformImplementation::type;
    using PlatformImplementation::width;
};
}

#endif // GFX_INTERFACES_TEXTURE_H