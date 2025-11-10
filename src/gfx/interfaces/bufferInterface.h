//
// Created by anton on 11/1/25.
//

#ifndef CYCLONITE_GFX_BUFFER_INTERFACE_H
#define CYCLONITE_GFX_BUFFER_INTERFACE_H

#include "core/resourceBase.h"
#include "gfx/common.h"
#include <concepts>

namespace cyclonite::gfx::interfaces {
template<typename T>
concept BufferConcept = requires(T t) {
    { t.usage() } -> std::same_as<BufferUsageFlagBits>;

    { t.map() } -> std::same_as<void*>;

    { t.unmap() } -> std::same_as<void>;
};

template<BufferConcept PlatformImplementation>
class BufferInterface : private PlatformImplementation
{
public:
    friend class core::ResourceBase;

    using PlatformImplementation::map;
    using PlatformImplementation::PlatformImplementation;
    using PlatformImplementation::resourceBase;
    using PlatformImplementation::unmap;
    using PlatformImplementation::usage;
};
}

#endif // CYCLONITE_GFX_BUFFER_INTERFACE_H