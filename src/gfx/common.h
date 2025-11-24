//
// Created by anton on 6/12/25.
//

#ifndef GFX_COMMON_H
#define GFX_COMMON_H

#include "formats.h"
#include <SDL3/SDL_video.h>
#include <common.h>
#include <metrix/enum.h>
#include <variant>

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

enum class PipelineType : uint_fast8_t
{
    Compute = 0,
    PrimitiveRasterization = 1,
    RayTracing = 2,
    GeometricShading = 3
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

#if defined(PLATFORM_WINDOWS)
#ifdef TRANSPARENT
#undef TRANSPARENT
#endif
#endif

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

enum class BorderColor : uint_fast8_t
{
    FLOAT_TRANSPARENT_BLACK = 0,
    INT_TRANSPARENT_BLACK = 1,
    FLOAT_OPAQUE_BLACK = 2,
    INT_OPAQUE_BLACK = 3,
    FLOAT_OPAQUE_WHITE = 4,
    INT_OPAQUE_WHITE = 5
};

enum class TextureFilter
{
    NEAREST = 0,
    LINEAR = 1
};

enum class TextureAddressMode
{
    REPEAT = 0,
    MIRRORED_REPEAT = 1,
    CLAMP_TO_EDGE = 2,
    CLAMP_TO_BORDER = 3,
    MIRROR_CLAMP_TO_EDGE = 4
};

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
    PREINITIALIZED = 8,
    DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL = 9,
    DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL = 10,
    DEPTH_ATTACHMENT_OPTIMAL = 11,
    DEPTH_READ_ONLY_OPTIMAL = 12,
    STENCIL_ATTACHMENT_OPTIMAL = 13,
    STENCIL_READ_ONLY_OPTIMAL = 14,
    READ_ONLY_OPTIMAL = 15,
    ATTACHMENT_OPTIMAL = 16
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

enum class BufferUsageFlags : uint32_t
{
    TRANSFER_SRC = 0x00000001,
    TRANSFER_DST = 0x00000002,
    UNIFORM_TEXEL_BUFFER = 0x00000004,
    STORAGE_TEXEL_BUFFER = 0x00000008,
    UNIFORM_BUFFER = 0x00000010,
    STORAGE_BUFFER = 0x00000020,
    INDEX_BUFFER = 0x00000040,
    VERTEX_BUFFER = 0x00000080,
    INDIRECT_BUFFER = 0x00000100,
    SHADER_DEVICE_ADDRESS = 0x00020000
};
using BufferUsageFlagBits = metrix::enum_bits<BufferUsageFlags>;

enum class PipelineCreationFlags : uint32_t
{
    DISABLE_OPTIMIZATION = 0x00000001,
    ALLOW_DERIVATIVES = 0x00000002,
    DERIVATIVE = 0x00000004,
    VIEW_INDEX_FROM_DEVICE = 0x00000008,
    DISPATCH_BASE = 0x00000010,
    FAIL_ON_PIPELINE_COMPILE_REQUIRED = 0x00000100,
    EARLY_RETURN_ON_FAILURE = 0x00000200
};
using PipelineCreationFlagBits = metrix::enum_bits<PipelineCreationFlags>;

enum class ShaderStageCreationFlags : uint32_t
{
    ALLOW_VARYING_SUBGROUP_SIZE = 0x00000001,
    REQUIRE_FULL_SUBGROUPS = 0x00000002
};
using ShaderStageCreationFlagBits = metrix::enum_bits<ShaderStageCreationFlags>;

enum class ShaderStageFlags : uint32_t
{
    VERTEX = 0x00000001,
    TESSELLATION_CONTROL = 0x00000002,
    TESSELLATION_EVALUATION = 0x00000004,
    GEOMETRY = 0x00000008,
    FRAGMENT = 0x00000010,
    COMPUTE = 0x00000020,
    ALL_GRAPHICS = 0x0000001F,
    ALL = 0x7FFFFFFF,
    RAYGEN = 0x00000100,
    ANY_HIT = 0x00000200,
    CLOSEST_HIT = 0x00000400,
    MISS = 0x00000800,
    INTERSECTION = 0x00001000,
    CALLABLE = 0x00002000,
    TASK = 0x00000040,
    MESH = 0x00000080,
    STAGE_COUNT = 14
};
using ShaderStageFlagBits = metrix::enum_bits<ShaderStageFlags>;

enum class RasterizationStateFlags : uint16_t
{
    DEPTH_CLAMP_ENABLE = 1 << 0,
    RASTERIZER_DISCARD_ENABLE = 1 << 1,
    DEPTH_BIAS_ENABLE = 1 << 2,
    DEPTH_TEST_ENABLE = 1 << 3,
    DEPTH_WRITE_ENABLE = 1 << 4,
    DEPTH_BOUNDS_TEST_ENABLE = 1 << 5,
    STENCIL_TEST_ENABLE = 1 << 6,
    BLEND_ENABLE = 1 << 7,
    BLEND_LOGICAL_OP_ENABLE = 1 << 8
};
using RasterizationStateFlagBits = metrix::enum_bits<RasterizationStateFlags>;

enum class PolygonMode : uint8_t
{
    FILL = 0,
    LINE = 1,
    POINT = 2
};

enum class CullMode : uint8_t
{
    NONE = 0,
    FRONT = 1,
    BACK = 2,
    FRONT_AND_BACK = 3
};

enum class FrontFace : uint8_t
{
    COUNTER_CLOCKWISE = 0,
    CLOCKWISE = 1
};

enum class CompareOp : uint8_t
{
    NEVER = 0,
    LESS = 1,
    EQUAL = 2,
    LESS_OR_EQUAL = 3,
    GREATER = 4,
    NOT_EQUAL = 5,
    GREATER_OR_EQUAL = 6,
    ALWAYS = 7
};

enum class StencilOp : uint8_t
{
    KEEP = 0,
    ZERO = 1,
    REPLACE = 2,
    INCREMENT_AND_CLAMP = 3,
    DECREMENT_AND_CLAMP = 4,
    INVERT = 5,
    INCREMENT_AND_WRAP = 6,
    DECREMENT_AND_WRAP = 7
};

enum class DescriptorType : uint_fast8_t
{
    SAMPLER = 0,
    COMBINED_IMAGE_SAMPLER = 1,
    SAMPLED_IMAGE = 2,
    STORAGE_IMAGE = 3,
    UNIFORM_TEXEL_BUFFER = 4,
    STORAGE_TEXEL_BUFFER = 5,
    UNIFORM_BUFFER = 6,
    STORAGE_BUFFER = 7,
    UNIFORM_BUFFER_DYNAMIC = 8,
    STORAGE_BUFFER_DYNAMIC = 9,
    INPUT_ATTACHMENT = 10,
    INLINE_UNIFORM_BLOCK = 11
};

enum class DescriptorSetLayoutFlags : uint32_t
{
    PUSH_DESCRIPTOR = 0x00000001,
    UPDATE_AFTER_BIND = 0x00000002
};
using DescriptorSetLayoutFlagBits = metrix::enum_bits<DescriptorSetLayoutFlags>;

enum class BindingFlags : uint32_t
{
    UPDATE_AFTER_BIND = 0x00000001,
    UPDATE_UNUSED_WHILE_PENDING = 0x00000002,
    PARTIALLY_BOUND = 0x00000004,
    VARIABLE_DESCRIPTOR_COUNT = 0x00000008
};
using BindingFlagBits = metrix::enum_bits<BindingFlags>;

enum class PipelineBindPoint : uint_fast8_t
{
    GRAPHICS = 0,
    COMPUTE = 1
};

enum class IndexType : uint_fast8_t
{
    TYPE_UINT16 = 0,
    TYPE_UINT32 = 1
};

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

enum class PrimitiveTopology : uint_fast8_t
{
    POINT_LIST = 0,
    LINE_LIST = 1,
    LINE_STRIP = 2,
    TRIANGLE_LIST = 3,
    TRIANGLE_STRIP = 4,
    TRIANGLE_FAN = 5,
    LINE_LIST_WITH_ADJACENCY = 6,
    LINE_STRIP_WITH_ADJACENCY = 7,
    TRIANGLE_LIST_WITH_ADJACENCY = 8,
    TRIANGLE_STRIP_WITH_ADJACENCY = 9,
    PATCH_LIST = 10
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
using QueueSubmissionStateFlagBits = metrix::enum_bits<QueueSubmissionStateFlags>;

struct PushConstantRange
{
    uint32_t size;
    uint32_t offset;
    ShaderStageFlagBits stage;
};

struct ResourceDescription
{
    DescriptorType type;
};

struct BufferResourceDescription : public ResourceDescription
{
    size_t offset;
    size_t size;
};

struct TextureResourceDescription : public ResourceDescription
{
    TextureState state;
};
}

#endif // GFX_COMMON_H
