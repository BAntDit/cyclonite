//
// Created by anton on 6/12/25.
//

#ifndef GFX_INTERFACES_INSTANCE_H
#define GFX_INTERFACES_INSTANCE_H

#include "core/resourceRef.h"
#include <concepts>
#include <metrix/type_traits.h>
#include <utility>

namespace cyclonite::gfx::interfaces {
template<typename T>
concept InstanceConcept = requires(T t)
{
    {
        t.physicalDeviceCount()
    }
    ->std::same_as<uint32_t>;

    requires std::is_member_function_pointer_v<decltype(&T::createDevice)>;

    requires std::is_same_v<core::ResourceRef, metrix::member_function_return_type_t<decltype(&T::createDevice)>>;
};

template<InstanceConcept InstanceImplementation>
class InstanceInterface : private InstanceImplementation
{
public:
    using InstanceImplementation::createDevice;
    using InstanceImplementation::InstanceImplementation;
    using InstanceImplementation::physicalDeviceCount;
};
}

#endif // GFX_INTERFACES_INSTANCE_H
