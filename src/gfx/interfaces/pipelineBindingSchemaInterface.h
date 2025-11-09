//
// Created by anton on 11/9/25.
//

#ifndef CYCLONITE_PIPELINE_BINDING_SCHEMA_INTERFACE_H
#define CYCLONITE_PIPELINE_BINDING_SCHEMA_INTERFACE_H

#include "core/resourceBase.h"
#include "gfx/common.h"
#include <concepts>

namespace cyclonite::gfx::interfaces {
template<typename T>
concept PipelineBindingSchemaConcept = requires(T t) {
    { t.descriptorSetLayoutCount() } -> std::same_as<uint32_t>;
};

template<PipelineBindingSchemaConcept PlatformImplementation>
class PipelineBindingSchemaInterface : private PlatformImplementation
{
public:
    friend class core::ResourceBase;

    using PlatformImplementation::descriptorSetLayoutCount;
    using PlatformImplementation::PlatformImplementation;
    using PlatformImplementation::resourceBase;
};
}

#endif // CYCLONITE_PIPELINE_BINDING_SCHEMA_INTERFACE_H