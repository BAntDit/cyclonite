

#ifndef CYCLONITE_GFX_DRAW_INDEXED_INDIRECT_COMMANDS_INTERFACE_H
#define CYCLONITE_GFX_DRAW_INDEXED_INDIRECT_COMMANDS_INTERFACE_H

#include "core/resourceSharedRef.h"
#include "gfx/common.h"
#include <concepts>

namespace cyclonite::gfx::interfaces {
template<typename T>
concept DrawIndexedIndirectCommandsConcept = requires(T& t, T const& ct, size_t s, int32_t o) {
    { ct.commandCount() } -> std::same_as<uint32_t>;

    { ct.capacity() } -> std::same_as<uint32_t>;

    { ct.state() } -> std::same_as<DrawIndirectCommandState>;

    { t.state() } -> std::same_as<DrawIndirectCommandState&>;

    { ct.staging() } -> std::same_as<core::ResourceSharedRef const&>;

    { ct.indexCount(s) } -> std::same_as<uint32_t>;

    { t.indexCount(s) } -> std::same_as<uint32_t&>;

    { ct.firstIndex(s) } -> std::same_as<uint32_t>;

    { t.firstIndex(s) } -> std::same_as<uint32_t&>;

    { ct.vertexOffset(s) } -> std::same_as<int32_t>;

    { t.vertexOffset(s) } -> std::same_as<int32_t&>;

    { ct.instanceCount(s) } -> std::same_as<uint32_t>;

    { t.instanceCount(s) } -> std::same_as<uint32_t&>;

    { ct.firstInstance(s) } -> std::same_as<uint32_t>;

    { t.firstInstance(s) } -> std::same_as<uint32_t&>;

    { t.addCommand(s, s, o, s, s) } -> std::same_as<void>;

    { t.validateStaging() } -> std::same_as<void>;
};

template<DrawIndexedIndirectCommandsConcept PlatformImplementation>
class DrawIndexedIndirectCommandsInterface : private PlatformImplementation
{
    using PlatformImplementation::PlatformImplementation;

    using PlatformImplementation::addCommand;
    using PlatformImplementation::capacity;
    using PlatformImplementation::commandCount;
    using PlatformImplementation::firstIndex;
    using PlatformImplementation::firstInstance;
    using PlatformImplementation::indexCount;
    using PlatformImplementation::instanceCount;
    using PlatformImplementation::staging;
    using PlatformImplementation::state;
    using PlatformImplementation::validateStaging;
    using PlatformImplementation::vertexOffset;
};
}

#endif // CYCLONITE_GFX_DRAW_INDEXED_INDIRECT_COMMANDS_INTERFACE_H
