//
// Created by anton on 6/12/25.
//

#ifndef GFX_COMMON_H
#define GFX_COMMON_H

#include <SDL3/SDL_video.h>
#include <cstdint>
#include <metrix/enum.h>
#include "formats.h"

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

enum class DeviceVendor : uint32_t
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

enum class SurfaceFlag : uint64_t
{
    FULLSCREEN = SDL_WINDOW_FULLSCREEN,
    HIDDEN = SDL_WINDOW_HIDDEN,
    BORDERLESS = SDL_WINDOW_BORDERLESS,
    RESIZABLE = SDL_WINDOW_RESIZABLE,
    MINIMIZED = SDL_WINDOW_MINIMIZED,
    MAXIMIZED = SDL_WINDOW_MAXIMIZED,
    HIGH_PIXEL_DENSITY = SDL_WINDOW_HIGH_PIXEL_DENSITY,
    ALWAYS_ON_TOP = SDL_WINDOW_ALWAYS_ON_TOP,
    TRANSPARENT = SDL_WINDOW_TRANSPARENT
};

using SurfaceFlagBits = metrix::enum_bits<SurfaceFlag>;

enum class TextureType : uint_fast8_t
{
    TEXTURE_1D = 0,
    TEXTURE_2D = 1,
    TEXTURE_3D = 2,
    TEXTURE_CUBE = 3,
    TEXTURE_1D_ARRAY = 4,
    TEXTURE_2D_ARRAY = 5,
    TEXTURE_CUBE_ARRAY = 6
};

enum class TextureTiling : uint_fast8_t
{
    OPTIMAL = 0,
    LINEAR = 1
};
}

#endif // GFX_COMMON_H
