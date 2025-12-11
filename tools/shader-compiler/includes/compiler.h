//
// Created by anton on 11/29/25.
//

#ifndef CYCLONITE_TOOLS_COMPILER_H
#define CYCLONITE_TOOLS_COMPILER_H

#if !defined(_WIN32) // _WIN32 / _WIN64 at once
#include <dxc/dxcapi.h>
#else
#include <dxcapi.h>
#include <windows.h>
#endif

namespace cyclonite::tools {
class Compiler
{
public:
    Compiler();

    Compiler(Compiler const&) = delete;

    Compiler(Compiler&& compiler) noexcept;

    ~Compiler();

    auto operator=(Compiler const&) -> Compiler& = delete;

    auto operator=(Compiler&& rhs) noexcept -> Compiler&;

private:
    IDxcLibrary* dxcLibrary_;
    IDxcUtils* dxcUtils_;
    IDxcCompiler* dxcCompiler_;
};
}

#endif // CYCLONITE_TOOLS_COMPILER_H