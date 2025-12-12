//
// Created by anton on 12/11/25.
//

#ifndef TOOLS_SHADER_COMPILER_COMMON_H
#define TOOLS_SHADER_COMPILER_COMMON_H

#include <cstdint>

namespace cyclonite::tools {
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

enum class DiagnosticMessageFormat: uint_fast8_t
{
    Undefined = 0,
    Clang = 1,
    Vi = 2,
    Msvc = 3,
    MdvcFallBack = 4
};

enum class OptionValue: uint_fast8_t
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

enum class Optimization: uint_fast8_t
{
    Disable = 0, // Od
    Level0 = 1, // O0
    Level1 = 2, // O1
    Level2 = 3, // O2
    Level3 = 4  // O3
};

enum class SpvDebug: uint_fast8_t
{
    Undefined = 0,
    VulkanWithSource = 1,
    File = 2, 
    Source = 3,
    Line = 4
};

enum class SpvTargetEnv: uint_fast8_t
{
    Default = 0,
    Vulkan_1_0 = 1,
    Vulkan_1_1 = 2,
    Vulkan_1_1_Spirv_1_4 = 2,
    Vulkan_1_2 = 3,
    Vulkan_1_3 = 4,
    Universal_1_5 = 5
};

enum class TargetProfile: uint_fast8_t
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
    hs_6_2 = 62, 
    hs_6_3 = 63, 
    hs_6_4 = 64, 
    hs_6_5 = 65, 
    hs_6_6 = 66, 
    hs_6_7 = 67, 
    hs_6_8 = 68, 
    hs_6_9 = 69, 
    ds_6_0 = 70,
    ds_6_1 = 71, 
    ds_6_2 = 72, 
    ds_6_3 = 73, 
    ds_6_4 = 74, 
    ds_6_5 = 75, 
    ds_6_6 = 76, 
    ds_6_7 = 77, 
    ds_6_8 = 78, 
    ds_6_9 = 79,
    cs_6_0 = 80, 
    cs_6_1 = 81, 
    cs_6_2 = 82, 
    cs_6_3 = 83, 
    cs_6_4 = 84, 
    cs_6_5 = 85, 
    cs_6_6 = 86, 
    cs_6_7 = 87, 
    cs_6_8 = 88,
    cs_6_9 = 89, 
    lib_6_1 = 90, 
    lib_6_2 = 91, 
    lib_6_3 = 92, 
    lib_6_4 = 93, 
    lib_6_5 = 94, 
    lib_6_6 = 95, 
    lib_6_7 = 96, 
    lib_6_8 = 97, 
    lib_6_9 = 98, 
    ms_6_5 = 99, 
    ms_6_6 = 100, 
    ms_6_7 = 101, 
    ms_6_8 = 102, 
    ms_6_9 = 103, 
    as_6_5 = 104, 
    as_6_6 = 105,
    as_6_7 = 106, 
    as_6_8 = 107, 
    as_6_9 = 108
};
}

#endif // TOOLS_SHADER_COMPILER_COMMON_H