//
// Created by anton on 11/8/25.
//

#ifndef CYCLONITE_BINDING_INTERFACE_H
#define CYCLONITE_BINDING_INTERFACE_H

#include "gfx/common.h"
#include <concepts>

namespace cyclonite::gfx::interfaces {
template<typename T>
concept BindingConcept = requires(T t) {
    { t.set() } -> std::same_as<uint32_t>;

    { t.binding() } -> std::same_as<uint32_t>;

    { t.descriptorType() } -> std::same_as<DescriptorType>;

    { t.stageFlags() } -> std::same_as<ShaderStageFlagBits>;

    { t.descriptorSetFlags() } -> std::same_as<DescriptorSetFlagBits>;

    { t.bindingFlags() } -> std::same_as<BindingFlagBits>;
};

template<BindingConcept PlatformImplementation>
class BindingInterface : private PlatformImplementation
{
public:
    using PlatformImplementation::binding;
    using PlatformImplementation::bindingFlags;
    using PlatformImplementation::descriptorSetFlags;
    using PlatformImplementation::descriptorType;
    using PlatformImplementation::PlatformImplementation;
    using PlatformImplementation::set;
    using PlatformImplementation::stageFlags;
};
}

#endif // CYCLONITE_BINDING_INTERFACE_H