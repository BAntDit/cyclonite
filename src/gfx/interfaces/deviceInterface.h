//
// Created by anton on 6/16/25.
//

#ifndef GFX_INTERFACES_DEVICE_H
#define GFX_INTERFACES_DEVICE_H

#include <string>

namespace cyclonite::gfx::interfaces {
template<typename T>
concept DeviceConcept = true;

template<DeviceConcept PlatformImplementation>
class DeviceInterface : private PlatformImplementation
{
    using PlatformImplementation::PlatformImplementation;
};
}

#endif // GFX_INTERFACES_DEVICE_H
