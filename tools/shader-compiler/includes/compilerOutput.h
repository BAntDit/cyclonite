//
// Created by anton on 12/26/25.
//

#ifndef TOOLS_SHADER_COMPILER_OUTPUT_H
#define TOOLS_SHADER_COMPILER_OUTPUT_H

#include "shaderReflection.h"

namespace cyclonite::tools {
class CompilerOutput
{
public:
    CompilerOutput() = default;

    [[nodiscard]] auto reflectionData() const -> shared::ShaderReflectionData const& { return reflectionData_; }

    [[nodiscard]] auto reflectionData() -> shared::ShaderReflectionData& { return reflectionData_; }

    [[nodiscard]] auto spirvModule() const -> std::vector<uint32_t> const& { return spirvModule_; }

    [[nodiscard]] auto spirvModule() -> std::vector<uint32_t>& { return spirvModule_; }

private:
    shared::ShaderReflectionData reflectionData_;
    std::vector<uint32_t> spirvModule_;
};
}

#endif // TOOLS_SHADER_COMPILER_OUTPUT_H