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
}

#endif // TOOLS_SHADER_COMPILER_COMMON_H