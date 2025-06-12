//
// Created by anton on 6/12/25.
//

#ifndef GFX_INSTANCE_INTERFACE_H
#define GFX_INSTANCE_INTERFACE_H

#include <concepts>

namespace cyclonite::gfx::interfaces {
template<typename T>
concept InstanceConcept = true;

template<InstanceConcept InstanceImplementation>
class InstanceInterface : private InstanceImplementation
{};
}

#endif // GFX_INSTANCE_INTERFACE_H
