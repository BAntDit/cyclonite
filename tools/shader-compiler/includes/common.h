//
// Created by anton on 12/11/25.
//

#ifndef TOOLS_SHADER_COMPILER_COMMON_H
#define TOOLS_SHADER_COMPILER_COMMON_H

#include <cstdint>

namespace cyclonite::tools {
enum class TargetPlatform : uint_fast8_t
{
    All = 0,
    Nix = 1,
    Windows = 2
};

enum class TargetGAPI : uint_fast8_t
{
    All = 0,
    Vulkan = 1,
    D3D12 = 2
};

enum class LinkageType : uint_fast8_t
{
    Undefined = 0,
    Internal = 1,
    External = 2
};

enum class Encoding : uint_fast8_t
{
    Undefined = 0,
    Utf8 = 1,
    Utf16Win = 2,
    Utf32Nix = 3,
    Wide = 4
};

enum class DiagnosticMessageFormat : uint_fast8_t
{
    Undefined = 0,
    Clang = 1,
    Vi = 2,
    Msvc = 3,
    MdvcFallBack = 4
};

enum class OptionValue : uint_fast8_t
{
    Default = 0,
    Enable = 1,
    Disable = 2
};

enum class HV : uint_fast8_t
{
    Undefined = 0,
    _2016 = 1,
    _2017 = 2,
    _2018 = 3,
    _2021 = 4
};

enum class Optimization : uint_fast8_t
{
    Disable = 0, // Od
    Level0 = 1,  // O0
    Level1 = 2,  // O1
    Level2 = 3,  // O2
    Level3 = 4   // O3
};

enum class SpvDebug : uint_fast8_t
{
    Undefined = 0,
    VulkanWithSource = 1,
    File = 2,
    Source = 3,
    Line = 4
};

enum class SpvTargetEnv : uint_fast8_t
{
    Default = 0,
    Vulkan_1_0 = 1,
    Vulkan_1_1 = 2,
    Vulkan_1_1_Spirv_1_4 = 3,
    Vulkan_1_2 = 4,
    Vulkan_1_3 = 5,
    Universal_1_5 = 6
};

enum class TargetProfile : uint_fast8_t
{
    ps_6_0 = 0,
    ps_6_1 = 1,
    ps_6_2 = 2,
    ps_6_3 = 3,
    ps_6_4 = 4,
    ps_6_5 = 5,
    ps_6_6 = 6,
    ps_6_7 = 7,
    ps_6_8 = 8,
    ps_6_9 = 9,
    vs_6_0 = 10,
    vs_6_1 = 11,
    vs_6_2 = 12,
    vs_6_3 = 13,
    vs_6_4 = 14,
    vs_6_5 = 15,
    vs_6_6 = 16,
    vs_6_7 = 17,
    vs_6_8 = 18,
    vs_6_9 = 19,
    gs_6_0 = 20,
    gs_6_1 = 21,
    gs_6_2 = 22,
    gs_6_3 = 23,
    gs_6_4 = 24,
    gs_6_5 = 25,
    gs_6_6 = 26,
    gs_6_7 = 27,
    gs_6_8 = 28,
    gs_6_9 = 29,
    hs_6_0 = 30,
    hs_6_1 = 31,
    hs_6_2 = 32,
    hs_6_3 = 33,
    hs_6_4 = 34,
    hs_6_5 = 35,
    hs_6_6 = 36,
    hs_6_7 = 37,
    hs_6_8 = 38,
    hs_6_9 = 39,
    ds_6_0 = 40,
    ds_6_1 = 41,
    ds_6_2 = 42,
    ds_6_3 = 43,
    ds_6_4 = 44,
    ds_6_5 = 45,
    ds_6_6 = 46,
    ds_6_7 = 47,
    ds_6_8 = 48,
    ds_6_9 = 49,
    cs_6_0 = 50,
    cs_6_1 = 51,
    cs_6_2 = 52,
    cs_6_3 = 53,
    cs_6_4 = 54,
    cs_6_5 = 55,
    cs_6_6 = 56,
    cs_6_7 = 57,
    cs_6_8 = 58,
    cs_6_9 = 59,
    lib_6_1 = 60,
    lib_6_2 = 61,
    lib_6_3 = 62,
    lib_6_4 = 63,
    lib_6_5 = 64,
    lib_6_6 = 65,
    lib_6_7 = 66,
    lib_6_8 = 67,
    lib_6_9 = 68,
    ms_6_5 = 69,
    ms_6_6 = 70,
    ms_6_7 = 71,
    ms_6_8 = 72,
    ms_6_9 = 73,
    as_6_5 = 74,
    as_6_6 = 75,
    as_6_7 = 76,
    as_6_8 = 77,
    as_6_9 = 78
};

enum class MatrixLayout : uint_fast8_t
{
    Undefined = 0,
    ColumnMajor = 1,
    RowMajor = 2
};
}

#endif // TOOLS_SHADER_COMPILER_COMMON_H