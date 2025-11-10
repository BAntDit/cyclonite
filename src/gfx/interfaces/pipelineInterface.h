//
// Created by anton on 11/10/25.
//

#ifndef CYCLONITE_PIPELINE_INTERFACE_H
#define CYCLONITE_PIPELINE_INTERFACE_H

#include "core/resourceBase.h"
#include "gfx/common.h"
#include <concepts>

namespace cyclonite::gfx::interfaces {
template<typename T>
concept PipelineConcept = requires(T t) {
    { t.type() } -> std::same_as<PipelineType>;
};

template<PipelineConcept PlatformImplementation>
class PipelineInterface : private PlatformImplementation
{
public:
    friend class core::ResourceBase;

    using PlatformImplementation::PlatformImplementation;
    using PlatformImplementation::resourceBase;
    using PlatformImplementation::type;
};
}

#endif // CYCLONITE_PIPELINE_INTERFACE_H