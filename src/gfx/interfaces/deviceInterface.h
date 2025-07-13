//
// Created by anton on 6/16/25.
//

#ifndef GFX_INTERFACES_DEVICE_H
#define GFX_INTERFACES_DEVICE_H

#include <concepts>

namespace cyclonite::core {
class ResourceBase;
}

namespace cyclonite::gfx::interfaces {
template<typename T>
concept DeviceConcept = requires(T t) {
    { t.resourceBase() } -> std::same_as<core::ResourceBase*>;
};

template<DeviceConcept PlatformImplementation>
class DeviceInterface : private PlatformImplementation
{
public:
    using PlatformImplementation::PlatformImplementation;
    using PlatformImplementation::resourceBase;
};
}

#endif // GFX_INTERFACES_DEVICE_H
