//
// Created by anton on 6/16/25.
//

#ifndef GFX_INTERFACES_DEVICE_H
#define GFX_INTERFACES_DEVICE_H

#include "core/resourceRef.h"
#include "gfx/common.h"
#include <concepts>
#include <string_view>
#include <utility>
#include <metrix/type_traits.h>

namespace cyclonite::gfx::interfaces {
template<typename T>
concept DeviceConcept = requires(T t, uint32_t a, uint32_t b, std::string_view s, SurfaceFlagBits f)
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
        t.limits()
    }
    ->std::same_as<DeviceLimits const&>;
    {
        t.createRenderWindow(a, b, s, f)
    }
    ->std::same_as<core::ResourceRef>;

    requires std::is_member_function_pointer_v<decltype(&T::createRenderPassWithRTVs)>;

    requires std::is_same_v<core::ResourceRef,
                            metrix::member_function_return_type_t<decltype(&T::createRenderPassWithRTVs)>>;
};

template<DeviceConcept PlatformImplementation>
class DeviceInterface : private PlatformImplementation
{
public:
    friend class core::ResourceBase;

    using PlatformImplementation::createRenderWindow;
    using PlatformImplementation::limits;
    using PlatformImplementation::name;
    using PlatformImplementation::PlatformImplementation;
    using PlatformImplementation::resourceBase;
    using PlatformImplementation::vendor;
};
}

#endif // GFX_INTERFACES_DEVICE_H
