//
// Created by anton on 12/22/25.
//

#ifndef TOOLS_SHADER_COMPILER_DXREFLECTION_H
#define TOOLS_SHADER_COMPILER_DXREFLECTION_H
#include "shaderReflection.h"
#include <metrix/containers.h>
#include <metrix/enum.h>
#include <metrix/type_traits.h>

struct ID3D12ShaderReflection;
struct ID3D12ShaderReflectionConstantBuffer;

namespace cyclonite::tools {
void collectReflection(ID3D12ShaderReflection* dxcShaderReflection, shared::ShaderReflectionData& reflectionData);
}

#endif // TOOLS_SHADER_COMPILER_DXREFLECTION_H