//
// Created by anton on 6/16/25.
//

#ifndef GFX_INTERFACES_DEVICE_H
#define GFX_INTERFACES_DEVICE_H

#include "core/resourceSharedRef.h"
#include "gfx/common.h"
#include <concepts>
#include <metrix/type_list.h>
#include <metrix/type_traits.h>
#include <string_view>

namespace cyclonite::gfx::interfaces {
template<typename T>
concept DeviceConcept = requires(T t,
                                 uint32_t a,
                                 uint32_t b,
                                 uint64_t c,
                                 std::string_view s,
                                 SurfaceFlagBits f,
                                 SignalType st,
                                 CommandPoolFlagBits cp) {
    { t.resourceBase() } -> std::same_as<core::ResourceBase*>;
    { t.name() } -> std::same_as<std::string_view>;
    { t.vendor() } -> std::same_as<DeviceVendor>;
    { t.limits() } -> std::same_as<DeviceLimits const&>;
    { t.createRenderWindow(a, b, s, f) } -> std::same_as<core::ResourceUniqueRef>;

    { t.createSignal(st, c) } -> std::same_as<core::ResourceUniqueRef>;

    { t.createCommandPool(a, cp) } -> std::same_as<core::ResourceUniqueRef>;

    requires std::is_member_function_pointer_v<decltype(&T::createRenderPassWithRenderWindow)>;

    requires std::is_same_v<core::ResourceUniqueRef,
                            metrix::member_function_return_type_t<decltype(&T::createRenderPassWithRenderWindow)>>;

    requires std::is_member_function_pointer_v<decltype(&T::createRenderPassWithRTVs)>;

    requires std::is_same_v<core::ResourceUniqueRef,
                            metrix::member_function_return_type_t<decltype(&T::createRenderPassWithRTVs)>>;

    requires std::is_member_function_pointer_v<decltype(&T::createTexture)>;

    requires std::is_same_v<core::ResourceUniqueRef,
                            metrix::member_function_return_type_t<decltype(&T::createTexture)>>;

    requires std::is_same_v<metrix::type_list<GpuMemoryAllocationFlagBits,
                                              TextureCreationFlagBits,
                                              TextureType,
                                              Format,
                                              uint32_t,
                                              uint32_t,
                                              uint32_t,
                                              uint32_t,
                                              uint32_t,
                                              TextureTiling,
                                              TextureUsageFlagBits>,
                            metrix::member_function_argument_type_list_t<decltype(&T::createTexture)>>;

    requires std::is_member_function_pointer_v<decltype(&T::createBuffer)> &&
               std::is_same_v<core::ResourceUniqueRef,
                              metrix::member_function_return_type_t<decltype(&T::createBuffer)>> &&
               std::is_same_v<metrix::type_list<GpuMemoryAllocationFlagBits, BufferUsageFlagBits, size_t>,
                              metrix::member_function_argument_type_list_t<decltype(&T::createBuffer)>>;
};

template<DeviceConcept PlatformImplementation>
class DeviceInterface : private PlatformImplementation
{
public:
    friend class core::ResourceBase;

    using PlatformImplementation::createBuffer;
    using PlatformImplementation::createCommandPool;
    using PlatformImplementation::createRenderWindow;
    using PlatformImplementation::createSignal;
    using PlatformImplementation::createTexture;
    using PlatformImplementation::limits;
    using PlatformImplementation::name;
    using PlatformImplementation::PlatformImplementation;
    using PlatformImplementation::resourceBase;
    using PlatformImplementation::vendor;
};
}

#endif // GFX_INTERFACES_DEVICE_H
