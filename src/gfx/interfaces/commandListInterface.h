//
// Created by anton on 9/28/25.
//

#ifndef CYCLONITE_GFX_COMMAND_LIST_INTERFACE_H
#define CYCLONITE_GFX_COMMAND_LIST_INTERFACE_H

#include "core/resourceSharedRef.h"
#include "gfx/common.h"
#include <concepts>

namespace cyclonite::gfx::interfaces {
template<typename T>
concept CommandListConcept = requires(T t, CommandListUsageFlagBits usage, core::ResourceSharedRef ref) {
    { t.state() } -> std::same_as<CommandListState>;
    { t.usage() } -> std::same_as<CommandListUsageFlagBits>;

    { t.begin(usage) } -> std::same_as<void>;

    { t.end() } -> std::same_as<void>;

    { t.beginRenderPass(ref) } -> std::same_as<void>;

    { t.endRenderPass() } -> std::same_as<void>;

    { t.bindPipeline(ref) } -> std::same_as<void>;
};

template<CommandListConcept PlatformImplementation>
class CommandListInterface : private PlatformImplementation
{
public:
    using PlatformImplementation::begin;
    using PlatformImplementation::beginRenderPass;
    using PlatformImplementation::bindPipeline;
    using PlatformImplementation::end;
    using PlatformImplementation::endRenderPass;
    using PlatformImplementation::PlatformImplementation;
    using PlatformImplementation::state;
    using PlatformImplementation::usage;

    // TODO:: refactor (adds platform_impl_mixin)
    [[nodiscard]] auto platformImplementation() const -> PlatformImplementation const& { return *this; }
    [[nodiscard]] auto platformImplementation() -> PlatformImplementation& { return *this; }
};
}

#endif // CYCLONITE_GFX_COMMAND_LIST_INTERFACE_H