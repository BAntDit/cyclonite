//
// Created by anton on 6/12/25.
//

#ifndef GFX_INTERFACES_INSTANCE_H
#define GFX_INTERFACES_INSTANCE_H

#include "resourceRef.h"
#include <concepts>
#include <utility>

namespace cyclonite::gfx::interfaces {
template<typename T>
concept InstanceConcept = requires(T t) {
                              {
                                  t.physicalDeviceCount()
                                  } -> std::same_as<uint32_t>;

                              requires std::is_member_function_pointer_v<decltype(&T::createDevice)>;

                              []<typename Ret, typename... Args>(Ret (T::*)(Args && ...)) constexpr -> bool {
                                  return std::is_same_v<Ret, gfx::ResourceRef>;
                              }(&T::createDevice);
                          };

template<InstanceConcept InstanceImplementation>
class InstanceInterface : private InstanceImplementation
{
public:
    using InstanceImplementation::physicalDeviceCount;
    using InstanceImplementation::createDevice;
};
}

#endif // GFX_INTERFACES_INSTANCE_H
