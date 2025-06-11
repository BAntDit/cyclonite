//
// Created by anton on 6/11/25.
//

#ifndef GFX_INTERFACES_SURFACE_H
#define GFX_INTERFACES_SURFACE_H
#include <cstdint>
#include <concepts>

namespace cyclonite::gfx::interfaces {
template<typename T>
concept SurfaceConcept = requires(T t)
{
    { t.width() } -> std::same_as<uint32_t>;
    { t.height() } -> std::same_as<uint32_t>;
};

template<SurfaceConcept PlatformImplementation>
class SurfaceInterface: private PlatformImplementation
{
public:
    using PlatformImplementation::width;
    using PlatformImplementation::height;
};
}

#endif // GFX_INTERFACES_SURFACE_H
