//
// Created by anton on 6/11/25.
//

#ifndef GFX_INTERFACES_SURFACE_H
#define GFX_INTERFACES_SURFACE_H
#include "core/resourceRef.h"
#include <concepts>
#include <cstdint>

namespace cyclonite::gfx::interfaces {
template<typename T>
concept SurfaceConcept = requires(T t) {
                             {
                                 t.width()
                                 } -> std::same_as<uint32_t>;
                             {
                                 t.height()
                                 } -> std::same_as<uint32_t>;
                         };

template<SurfaceConcept PlatformImplementation>
class SurfaceInterface : private PlatformImplementation
{
public:
    friend class core::ResourceBase;

    using PlatformImplementation::height;
    using PlatformImplementation::PlatformImplementation;
    using PlatformImplementation::width;
    using PlatformImplementation::resourceBase;
};
}

#endif // GFX_INTERFACES_SURFACE_H
