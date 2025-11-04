//
// Created by anton on 11/2/25.
//

#ifndef CYCLONITE_GFX_SHADER_INTERFACE_H
#define CYCLONITE_GFX_SHADER_INTERFACE_H

#include "core/resourceWeakRef.h"
#include "gfx/common.h"
#include <concepts>
#include <string>

namespace cyclonite::gfx::interfaces {
template<typename T>
concept ShaderConcept = requires(T t) {
    { t.creationFlags() } -> std::same_as<ShaderStageCreationFlagBits>;

    { t.stage() } -> std::same_as<ShaderStageFlags>;

    { t.entryPointName() } -> std::same_as<std::string_view>;
};

template<ShaderConcept PlatformImplementation>
class ShaderInterface : private PlatformImplementation
{
public:
    friend class core::ResourceBase;

    using PlatformImplementation::creationFlags;
    using PlatformImplementation::entryPointName;
    using PlatformImplementation::PlatformImplementation;
    using PlatformImplementation::resourceBase;
    using PlatformImplementation::stage;
};
}

#endif // CYCLONITE_GFX_SHADER_INTERFACE_H