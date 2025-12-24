//
// Created by anton on 12/22/25.
//

#ifndef TOOLS_SHADER_COMPILER_DXREFLECTION_H
#define TOOLS_SHADER_COMPILER_DXREFLECTION_H
#include "shaderReflection.h"
#include <cstdint>
#include <string>
#include <vector>

struct ID3D12ShaderReflection;

namespace cyclonite::tools {
class DxReflection
{
public:
    void getShaderDesc(ID3D12ShaderReflection* dxcShaderReflection);

private:
    shared::ShaderReflectionData reflectionData;
};
}

#endif // TOOLS_SHADER_COMPILER_DXREFLECTION_H