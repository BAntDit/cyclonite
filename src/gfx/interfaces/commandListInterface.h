//
// Created by anton on 9/28/25.
//

#ifndef CYCLONITE_GFX_COMMAND_LIST_INTERFACE_H
#define CYCLONITE_GFX_COMMAND_LIST_INTERFACE_H

#include "core/resourceSharedRef.h"
#include "gfx/common.h"
#include <concepts>
#include <span>

namespace cyclonite::gfx::interfaces {
template<typename T>
concept CommandListConcept = requires(T t,
                                      CommandListUsageFlagBits usage,
                                      core::ResourceSharedRef ref,
                                      PipelineBindPoint bindPoint,
                                      size_t a,
                                      IndexType indexType,
                                      uint32_t val,
                                      int32_t val2,
                                      std::span<uint32_t> ofs,
                                      PipelineStageFlagBits psf,
                                      AccessFlagBits afs) {
    { t.state() } -> std::same_as<CommandListState>;
    { t.usage() } -> std::same_as<CommandListUsageFlagBits>;

    { t.begin(usage) } -> std::same_as<void>;

    { t.end() } -> std::same_as<void>;

    { t.beginRenderPass(ref) } -> std::same_as<void>;

    { t.endRenderPass() } -> std::same_as<void>;

    { t.bindPipeline(ref) } -> std::same_as<void>;

    { t.bindDescriptorSet(bindPoint, ref, ref, ofs) } -> std::same_as<void>;

    { t.bindIndexBuffer(ref, a, indexType) } -> std::same_as<void>;

    { t.draw(val, val, val, val) } -> std::same_as<void>;

    { t.drawIndexed(val, val, val, val2, val) } -> std::same_as<void>;

    { t.drawIndirect(ref, a, val) } -> std::same_as<void>;

    { t.drawIndexedIndirect(ref, a, val) } -> std::same_as<void>;

    { t.acquireResourceForGraphics(psf, psf, afs, afs, ref, a, a) } -> std::same_as<void>;

    { t.acquireResourceForTransfer(psf, psf, afs, afs, ref, a, a) } -> std::same_as<void>;

    { t.releaseResourceToGraphics(psf, psf, afs, afs, ref, a, a) } -> std::same_as<void>;

    { t.copyBuffers(ref, ref, a, a, a) } -> std::same_as<void>;
};

template<CommandListConcept PlatformImplementation>
class CommandListInterface : private PlatformImplementation
{
public:
    using PlatformImplementation::acquireResourceForGraphics;
    using PlatformImplementation::acquireResourceForTransfer;
    using PlatformImplementation::begin;
    using PlatformImplementation::beginRenderPass;
    using PlatformImplementation::bindDescriptorSet;
    using PlatformImplementation::bindIndexBuffer;
    using PlatformImplementation::bindPipeline;
    using PlatformImplementation::copyBuffers;
    using PlatformImplementation::draw;
    using PlatformImplementation::drawIndexed;
    using PlatformImplementation::drawIndexedIndirect;
    using PlatformImplementation::drawIndirect;
    using PlatformImplementation::end;
    using PlatformImplementation::endRenderPass;
    using PlatformImplementation::PlatformImplementation;
    using PlatformImplementation::releaseResourceToGraphics;
    using PlatformImplementation::state;
    using PlatformImplementation::usage;

    // TODO:: refactor (adds platform_impl_mixin)
    [[nodiscard]] auto platformImplementation() const -> PlatformImplementation const& { return *this; }
    [[nodiscard]] auto platformImplementation() -> PlatformImplementation& { return *this; }
};
}

#endif // CYCLONITE_GFX_COMMAND_LIST_INTERFACE_H