//
// Created by anton on 6/16/25.
//

#ifndef GFX_INTERFACES_DEVICE_H
#define GFX_INTERFACES_DEVICE_H

#include "gfx/common.h"
#include <concepts>
#include <string_view>

namespace cyclonite::core {
class ResourceBase;
}

namespace cyclonite::gfx::interfaces {
template<typename T>
concept DeviceConcept = requires(T t) {
                            {
                                t.resourceBase()
                                } -> std::same_as<core::ResourceBase*>;
                            {
                                t.name()
                                } -> std::same_as<std::string_view>;
                            {
                                t.vendor()
                                } -> std::same_as<DeviceVendor>;
                        };

template<DeviceConcept PlatformImplementation>
class DeviceInterface : private PlatformImplementation
{
public:
    using PlatformImplementation::name;
    using PlatformImplementation::PlatformImplementation;
    using PlatformImplementation::resourceBase;
    using PlatformImplementation::vendor;
};
}

#endif // GFX_INTERFACES_DEVICE_H
