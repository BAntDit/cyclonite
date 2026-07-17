//
// Created by anton on 11/22/25.
//

#ifndef CYCLONITE_GFX_SAMPLER_INTERFACE_H
#define CYCLONITE_GFX_SAMPLER_INTERFACE_H

#include "core/resourceBase.h"
#include "gfx/common.h"
#include <concepts>

namespace cyclonite::gfx::interfaces {
template<typename T>
concept SamplerConcept = requires(T t) {
    { t.magFilter() } -> std::same_as<TextureFilter>;

    { t.minFilter() } -> std::same_as<TextureFilter>;

    { t.mipFilter() } -> std::same_as<TextureFilter>;

    { t.addressModeUVW() } -> std::same_as<std::tuple<TextureAddressMode, TextureAddressMode, TextureAddressMode>>;

    { t.maxAnisatropy() } -> std::same_as<real>;

    { t.mipLodBias() } -> std::same_as<real>;

    { t.maxLod() } -> std::same_as<real>;

    { t.minLod() } -> std::same_as<real>;
};

template<SamplerConcept PlatformImplementation>
class SamplerInterface : private PlatformImplementation
{
public:
    friend class core::ResourceBase;

    using PlatformImplementation::addressModeUVW;
    using PlatformImplementation::magFilter;
    using PlatformImplementation::maxAnisatropy;
    using PlatformImplementation::maxLod;
    using PlatformImplementation::minFilter;
    using PlatformImplementation::minLod;
    using PlatformImplementation::mipFilter;
    using PlatformImplementation::mipLodBias;
    using PlatformImplementation::PlatformImplementation;
    using PlatformImplementation::resourceBase;
};
}

#endif // CYCLONITE_GFX_SAMPLER_INTERFACE_H