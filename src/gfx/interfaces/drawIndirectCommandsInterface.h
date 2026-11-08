
#ifndef CYCLONITE_GFX_DRAW_INDIRECT_COMMANDS_INTERFACE_H
#define CYCLONITE_GFX_DRAW_INDIRECT_COMMANDS_INTERFACE_H

#include "core/resourceSharedRef.h"
#include "gfx/common.h"
#include <concepts>

namespace cyclonite::gfx::interfaces {
template<typename T>
concept DrawIndirectCommandsConcept = requires(T& t, T const& ct, size_t s) {
    { ct.commandCount() } -> std::same_as<uint32_t>;

    { ct.capacity() } -> std::same_as<uint32_t>;

    { ct.state() } -> std::same_as<DrawIndirectCommandState>;

    { t.state() } -> std::same_as<DrawIndirectCommandState&>;

    { ct.staging() } -> std::same_as<core::ResourceSharedRef const&>;

    { ct.vertexCount(s) } -> std::same_as<uint32_t>;

    { t.vertexCount(s) } -> std::same_as<uint32_t&>;

    { ct.firstVertex(s) } -> std::same_as<uint32_t>;

    { t.firstVertex(s) } -> std::same_as<uint32_t&>;

    { ct.instanceCount(s) } -> std::same_as<uint32_t>;

    { t.instanceCount(s) } -> std::same_as<uint32_t&>;

    { ct.firstInstance(s) } -> std::same_as<uint32_t>;

    { t.firstInstance(s) } -> std::same_as<uint32_t&>;

    { t.addCommand(s, s, s, s) } -> std::same_as<void>;

    { t.validateStaging() } -> std::same_as<void>;
};

template<DrawIndirectCommandsConcept PlatformImplementation>
class DrawIndirectCommandsInterface : private PlatformImplementation
{
public:
    using PlatformImplementation::PlatformImplementation;

    using PlatformImplementation::addCommand;
    using PlatformImplementation::capacity;
    using PlatformImplementation::commandCount;
    using PlatformImplementation::firstInstance;
    using PlatformImplementation::firstVertex;
    using PlatformImplementation::instanceCount;
    using PlatformImplementation::staging;
    using PlatformImplementation::state;
    using PlatformImplementation::validateStaging;
    using PlatformImplementation::vertexCount;
};
}

#endif // CYCLONITE_GFX_DRAW_INDIRECT_COMMANDS_INTERFACE_H
