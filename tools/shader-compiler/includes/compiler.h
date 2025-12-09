//
// Created by anton on 11/29/25.
//

#ifndef CYCLONITE_TOOLS_COMPILER_H
#define CYCLONITE_TOOLS_COMPILER_H

#include <dxc/dxcapi.h>

namespace cyclonite::tools {
class Compiler
{
public:
    Compiler();

    Compiler(Compiler const&) = delete;

    Compiler(Compiler&& compiler) noexcept;

    ~Compiler();

private:
    IDxcLibrary* dxcLibrary_;
    IDxcUtils* dxcUtils_;
    IDxcCompiler* dxcCompiler_;
};
}

#endif // CYCLONITE_TOOLS_COMPILER_H