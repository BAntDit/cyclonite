//
// Created by anton on 6/16/25.
//

#ifndef GFX_INTERFACES_DEVICE_H
#define GFX_INTERFACES_DEVICE_H

#include "core/resourceRef.h"
#include "gfx/common.h"
#include <concepts>
#include <string_view>

namespace cyclonite::core {
class ResourceBase;
}

namespace cyclonite::gfx::interfaces {
template<typename T>
concept DeviceConcept = requires(T t, uint32_t a, uint32_t b, std::string_view s, bool f)
{
    {
        t.resourceBase()
    }
    ->std::same_as<core::ResourceBase*>;
    {
        t.name()
    }
    ->std::same_as<std::string_view>;
    {
        t.vendor()
    }
    ->std::same_as<DeviceVendor>;
    {
        t.createSurface(a, b, s, f)
    }
    ->std::same_as<core::ResourceRef>;
};

template<DeviceConcept PlatformImplementation>
class DeviceInterface : private PlatformImplementation
{
public:
    using PlatformImplementation::createSurface;
    using PlatformImplementation::name;
    using PlatformImplementation::PlatformImplementation;
    using PlatformImplementation::resourceBase;
    using PlatformImplementation::vendor;
};
}

#endif // GFX_INTERFACES_DEVICE_H
