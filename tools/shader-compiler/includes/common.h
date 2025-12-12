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
}

#endif // TOOLS_SHADER_COMPILER_COMMON_H