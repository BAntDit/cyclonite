//
// Created by anton on 7/26/25.
//

#ifndef CYCLONITE_RENDERPASSINTERFACE_H
#define CYCLONITE_RENDERPASSINTERFACE_H

#include "core/resourceSharedRef.h"
#include "gfx/common.h"
#include <concepts>

namespace cyclonite::gfx::interfaces {
template<typename T>
concept RenderPassConcept = requires(T t) {
    { t.resourceBase() } -> std::same_as<core::ResourceBase*>;

    { t.width() } -> std::same_as<uint32_t>;

    { t.height() } -> std::same_as<uint32_t>;

    { t.isPresentationPass() } -> std::same_as<bool>;
};

template<RenderPassConcept PlatformImplementation>
class RenderPassInterface : private PlatformImplementation
{
public:
    friend class core::ResourceBase;

    using PlatformImplementation::height;
    using PlatformImplementation::isPresentationPass;
    using PlatformImplementation::PlatformImplementation;
    using PlatformImplementation::resourceBase;
    using PlatformImplementation::width;
};
}

#endif // CYCLONITE_RENDERPASSINTERFACE_H
