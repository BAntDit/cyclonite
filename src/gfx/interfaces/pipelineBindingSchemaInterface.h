//
// Created by anton on 11/9/25.
//

#ifndef CYCLONITE_PIPELINE_BINDING_SCHEMA_INTERFACE_H
#define CYCLONITE_PIPELINE_BINDING_SCHEMA_INTERFACE_H

#include "core/resourceBase.h"
#include <concepts>

namespace cyclonite::gfx::interfaces {
template<typename T>
concept PipelineBindingSchemaConcept = requires(T t) {
    {
        t.descriptorSetLayouts()
    } -> std::same_as<
        std::array<core::ResourceSharedRef, metrix::value_cast(DescriptorSpace::DESCRIPTOR_SPACE_COUNT)> const&>;
};

template<PipelineBindingSchemaConcept PlatformImplementation>
class PipelineBindingSchemaInterface : private PlatformImplementation
{
public:
    friend class core::ResourceBase;

    using PlatformImplementation::descriptorSetLayouts;
    using PlatformImplementation::PlatformImplementation;
    using PlatformImplementation::resourceBase;
};
}

#endif // CYCLONITE_PIPELINE_BINDING_SCHEMA_INTERFACE_H