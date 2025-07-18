//
// Created by anton on 6/12/25.
//

#ifndef GFX_COMMON_H
#define GFX_COMMON_H

namespace cyclonite::gfx {
namespace type_traits {
template<typename GfxType>
struct get_platform_implementation;

template<template<typename> class Interface, typename ImplementationType>
struct get_platform_implementation<Interface<ImplementationType>>
{
    using implementation_type = ImplementationType;
};

template<typename GfxType>
using platform_implementation_t = typename get_platform_implementation<GfxType>::implementation_type;
}

struct DeviceLimits
{
    bool supportCompute;
    bool dedicatedTransferQueue;
    bool dedicatedComputeQueue;
};

enum class DeviceVendor: uint32_t
{
    Unknown = 0,
    AMD = 0x1002, 
    NVIDIA = 0x10DE, 
    ImgTec_PowerVR = 0x1010,
    ARM_MaliGPU = 0x13B5,
    Apple = 0x106B,
    Intel = 0x8086,
    VIATechnologies = 0x10004,
    Vivante = 0x10005,
    VMware_virtualGPU = 0x10006,
    QEMU_emulatedGPU = 0x10007,
    Google_SwiftShader_virtualGPU = 0x1AE0,
    Qualcomm_AdrenoGPU = 0x5143
};
}

#endif // GFX_COMMON_H
