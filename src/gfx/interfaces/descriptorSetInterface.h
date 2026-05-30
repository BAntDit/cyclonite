//
// Created by anton on 11/16/25.
//

#ifndef CYCLONITE_GFX_DESCRIPTOR_SET_INTERFACE_H
#define CYCLONITE_GFX_DESCRIPTOR_SET_INTERFACE_H

#include "core/resourceBase.h"
#include "gfx/descriptorUpdateData.h"
#include <concepts>
#include <span>

namespace cyclonite::gfx::interfaces {
template<typename T>
concept DescriptorSetConcept = requires(T t,
                                        core::ResourceSharedRef r,
                                        std::span<DescriptorWriteData const> writeData,
                                        std::span<DescriptorCopyData const> copyData) {
    { t.index() } -> std::same_as<uint32_t>;

    { t.update(writeData) } -> std::same_as<void>;

    { t.copy(r, copyData) } -> std::same_as<void>;
};

template<DescriptorSetConcept PlatformImplementation>
class DescriptorSetInterface : private PlatformImplementation
{
public:
    friend class core::ResourceBase;

    using PlatformImplementation::copy;
    using PlatformImplementation::index;
    using PlatformImplementation::PlatformImplementation;
    using PlatformImplementation::resourceBase;
    using PlatformImplementation::update;
};
}

#endif // CYCLONITE_DESCRIPTOR_SET_INTERFACE_H