//
// Created by anton on 11/16/25.
//

#ifndef CYCLONITE_GFX_DESCRIPTOR_SET_INTERFACE_H
#define CYCLONITE_GFX_DESCRIPTOR_SET_INTERFACE_H

#include "core/resourceBase.h"
#include <concepts>

namespace cyclonite::gfx::interfaces {
template<typename T>
concept DescriptorSetConcept = requires(T t) {
    { t.index() } -> std::same_as<uint32_t>;
};

template<DescriptorSetConcept PlatformImplementation>
class DescriptorSetInterface : private PlatformImplementation
{
    friend class core::ResourceBase;

    using PlatformImplementation::index;
    using PlatformImplementation::PlatformImplementation;
    using PlatformImplementation::resourceBase;
};
}

#endif // CYCLONITE_DESCRIPTOR_SET_INTERFACE_H