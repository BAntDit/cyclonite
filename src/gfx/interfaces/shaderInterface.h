//
// Created by anton on 11/2/25.
//

#ifndef CYCLONITE_GFX_SHADER_INTERFACE_H
#define CYCLONITE_GFX_SHADER_INTERFACE_H

#include "core/resourceWeakRef.h"
#include "gfx/common.h"
#include <concepts>

namespace cyclonite::gfx::interfaces {
template<typename T>
concept ShaderConcept = requires(T t) {
    { t.id() } -> std::same_as<uint32_t>;
};

template<ShaderConcept PlatformImplementation>
class ShaderInterface : private PlatformImplementation
{
public:
    friend class core::ResourceBase;

    using PlatformImplementation::id;
    using PlatformImplementation::PlatformImplementation;
    using PlatformImplementation::resourceBase;
};
}

#endif // CYCLONITE_GFX_SHADER_INTERFACE_H