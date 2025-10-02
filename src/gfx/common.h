//
// Created by anton on 6/12/25.
//

#ifndef GFX_COMMON_H
#define GFX_COMMON_H

#include "formats.h"
#include <SDL3/SDL_video.h>
#include <metrix/enum.h>

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
    uint8_t maxColorAttachmentCount;
};

enum class PresentMode : uint_fast8_t
{
    Immediate = 0,
    MailBox = 1,
    FiFo = 2,
    FiFoRelaxed = 3
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

enum class SurfaceFlags : uint64_t
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

using SurfaceFlagBits = metrix::enum_bits<SurfaceFlags>;

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

enum class TextureState : uint_fast8_t
{
    UNDEFINED = 0,
    GENERAL = 1,
    COLOR_ATTACHMENT_OPTIMAL = 2,
    DEPTH_STENCIL_ATTACHMENT_OPTIMAL = 3,
    DEPTH_STENCIL_READ_ONLY_OPTIMAL = 4,
    SHADER_READ_ONLY_OPTIMAL = 5,
    TRANSFER_SRC_OPTIMAL = 6,
    TRANSFER_DST_OPTIMAL = 7,
    PREINITIALIZED = 8
};

enum class TextureCreationFlags : uint32_t
{
    SPARSE_BINDING = 0x00000001,
    SPARSE_RESIDENCY = 0x00000002,
    SPARSE_ALIASED = 0x00000004,
    MUTABLE_FORMAT = 0x00000008,
    CUBE_COMPATIBLE = 0x00000010
};

using TextureCreationFlagBits = metrix::enum_bits<TextureCreationFlags>;

enum class TextureUsageFlags : uint32_t
{
    TRANSFER_SRC = 0x00000001,
    TRANSFER_DST = 0x00000002,
    SAMPLED = 0x00000004,
    STORAGE = 0x00000008,
    COLOR_ATTACHMENT = 0x00000010,
    DEPTH_STENCIL_ATTACHMENT = 0x00000020,
    TRANSIENT_ATTACHMENT = 0x00000040,
    INPUT_ATTACHMENT = 0x00000080
};

using TextureUsageFlagBits = metrix::enum_bits<TextureUsageFlags>;

enum class GpuMemoryAllocationFlags : uint8_t
{
    DEDICATED_MEMORY = 1 << 0,
    USE_EXISTING_BLOCK = 1 << 1,
    PERSISTENT_MAPPED_MEMORY = 1 << 2,
    HOST_ACCESS_SEQUENTIAL_WRITE = 1 << 3,
    HOST_ACCESS_RANDOM_ORDER_WRITE_AND_READ = 1 << 4,
    MIN_MEMORY_STRATEGY = 1 << 5,
    MIN_TIME_STRATEGY = 1 << 6,
    MIN_OFFSET_STRATEGY = 1 << 7
};

using GpuMemoryAllocationFlagBits = metrix::enum_bits<GpuMemoryAllocationFlags>;

inline constexpr auto isColorFormat(Format format) -> bool
{
    auto v = metrix::value_cast(format);
    return ((v > 0 && v <= 123) || (v >= 131));
}

inline constexpr auto isStencilFormat(Format format) -> bool
{
    return format == Format::S8_UINT || format == Format::D16_UNORM_S8_UINT || format == Format::D24_UNORM_S8_UINT ||
           format == Format::D32_SFLOAT_S8_UINT;
}

inline constexpr auto isStencilOnlyFormat(Format format) -> bool
{
    return format == Format::S8_UINT;
}

enum class CommandPoolFlags : uint8_t
{
    TRANSIENT = 1 >> 0,
    ALLOW_COMMAND_BUFFERS_RESET = 1 >> 1
};

using CommandPoolFlagBits = metrix::enum_bits<CommandPoolFlags>;

enum class CommandListState : uint_fast8_t
{
    Initial = 0,
    Recording = 1,
    Executable = 2,
    Pending = 3,
    Invalid = 4
};

enum class CommandListUsageFlags : uint8_t
{
    ONE_TIME_SUBMIT = 1 << 0,
    RENDER_PASS_CONTINUE = 1 << 1,
    SIMULTANEOUS_USE = 1 << 2
};

using CommandListUsageFlagBits = metrix::enum_bits<CommandListUsageFlags>;

enum class SignalType : uint_fast8_t
{
    BINARY = 0,
    TIMELINE = 1
};

enum class PipelineStageFlags : uint32_t
{
    TOP_OF_PIPE_BIT = 1 << 0,
    DRAW_INDIRECT_BIT = 1 << 1,
    VERTEX_INPUT_BIT = 1 << 2,
    VERTEX_SHADER_BIT = 1 << 3,
    TESSELLATION_CONTROL_SHADER_BIT = 1 << 4,
    TESSELLATION_EVALUATION_SHADER_BIT = 1 << 5,
    GEOMETRY_SHADER_BIT = 1 << 6,
    FRAGMENT_SHADER_BIT = 1 << 7,
    EARLY_FRAGMENT_TEST_BIT = 1 << 8,
    LATE_FRAGMENT_TEST_BIT = 1 << 9,
    COLOR_ATTACHMENT_OUTPUT_BIT = 1 << 10,
    COMPUTE_SHADER_BIT = 1 << 11,
    TRANSFER_BIT = 1 << 12,
    BOTTOM_OF_PIPE_BIT = 1 << 13,
    HOST_BIT = 1 << 14,
    ALL_GRAPHICS_BIT = 1 << 15,
    ALL_COMMANDS_BIT = 1 << 16
};

using PipelineStageFlagBits = metrix::enum_bits<PipelineStageFlags>;


enum class QueueSubmissionStateFlags : uint32_t
{
    Invalid = 1 << 0,
    Initial = 1 << 1,
    BatchRecording = 1 << 2,
    CommandListRecording = 1 << 3,
    Recording = 1 << 4,
    Executable = 1 << 5,
    Pending = 1 << 6
};

}

#endif // GFX_COMMON_H
