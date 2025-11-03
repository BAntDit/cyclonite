
#ifndef CYCLONITE_GFX_SHADER_STAGE_INTERFACE_H
#define CYCLONITE_GFX_SHADER_STAGE_INTERFACE_H

#include "core/resourceWeakRef.h"
#include "gfx/common.h"
#include <concepts>
#include <string>

namespace cyclonite::gfx::interfaces 
{
template<typename T>
concept ShaderStageConcept = requires(T t) 
{
    { t.shader() } -> std::same_as<core::ResourceWeakRef>;

    { t.creationFlags() } -> std::same_as<ShaderStageCreationFlagBits>;

    { t.stage() } -> std::same_as<ShaderStageFlags>;

    { t.entryPointName() } -> std::same_as<std::string_view>;
};

template<ShaderStageConcept PlatformImplementation>
class ShaderStageInterface : private PlatformImplementation
{
public:
    using PlatformImplementation::PlatformImplementation;
    using PlatformImplementation::shader;
    using PlatformImplementation::creationFlags;
    using PlatformImplementation::stage;
    using PlatformImplementation::entryPointName;

    [[nodiscard]] auto platformImplementation() const -> PlatformImplementation const& { return *this; }
    [[nodiscard]] auto platformImplementation() -> PlatformImplementation& { return *this; }
};
}

#endif CYCLONITE_GFX_SHADER_STAGE_INTERFACE_H
