//
// Created by anton on 11/29/25.
//

#ifndef CYCLONITE_TOOLS_COMPILER_H
#define CYCLONITE_TOOLS_COMPILER_H
#include <shaderReflection.h>

#if !defined(_WIN32) // _WIN32 / _WIN64 at once
#include <dxc/dxcapi.h>
#else
#include <windows.h>
#include <dxcapi.h>
#include <initguid.h> 
#include <d3d12shader.h>
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif
#endif

#include "compilerInput.h"

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

    void collectReflection(std::wstring_view source,
                           Options const& options,
                           shared::ShaderReflectionData& reflectionData);

    void compileToSpirv(std::wstring_view source, Options const& options, std::vector<uint32_t>& output);

private:
    void compile(std::wstring_view source, Options const& options, IDxcResult*& compileResult);

private:
    IDxcLibrary* dxcLibrary_;
    IDxcUtils* dxcUtils_;
    IDxcCompiler3* dxcCompiler_;
};
}

#endif // CYCLONITE_TOOLS_COMPILER_H