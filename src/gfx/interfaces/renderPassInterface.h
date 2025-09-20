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
                                {
                                    t.resourceBase()
                                    } -> std::same_as<core::ResourceBase*>;
                                // TODO:: begin /end methods
                            };

template<RenderPassConcept PlatformImplementation>
class RenderPassInterface : private PlatformImplementation
{
public:
    friend class core::ResourceBase;

    using PlatformImplementation::PlatformImplementation;
    using PlatformImplementation::resourceBase;
    // begin / end
};
}

#endif // CYCLONITE_RENDERPASSINTERFACE_H
