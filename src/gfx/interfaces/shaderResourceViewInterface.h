//
// Created by anton on 11/23/25.
//

#ifndef CYCLONITE_SHADER_RESOURCE_VIEW_INTERFACE_H
#define CYCLONITE_SHADER_RESOURCE_VIEW_INTERFACE_H

#include "core/resourceWeakRef.h"
#include "gfx/common.h"
#include <concepts>

namespace cyclonite::gfx::interfaces {
template<typename T>
concept ShaderResourceViewConcept = requires(T t) {
    { t.texture() } -> std::same_as<core::ResourceWeakRef>;

    { t.maxMipLevel() } -> std::same_as<uint32_t>;

    { t.maxArrayLayer() } -> std::same_as<uint32_t>;
};

template<ShaderResourceViewConcept PlatformImplementation>
class ShaderResourceViewInterface : private PlatformImplementation
{
public:
    friend class core::ResourceBase;

    using PlatformImplementation::maxArrayLayer;
    using PlatformImplementation::maxMipLevel;
    using PlatformImplementation::PlatformImplementation;
    using PlatformImplementation::resourceBase;
    using PlatformImplementation::texture;
};
}

#endif // CYCLONITE_SHADER_RESOURCE_VIEW_INTERFACE_H