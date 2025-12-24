//
// Created by anton on 12/22/25.
//

#include "dxReflection.h"

#if !defined(_WIN32) // _WIN32 / _WIN64 at once
#include <directx/d3d12shader.h>
#include <unknwn.h>
#else
#include <d3d12shader.h>
#endif

#include <stdexcept>

namespace cyclonite::tools {
void DxReflection::getShaderDesc(ID3D12ShaderReflection* dxcShaderReflection)
{
    auto shaderDesc = D3D12_SHADER_DESC{};
    if (auto result = dxcShaderReflection->GetDesc(&shaderDesc); !SUCCEEDED(result)) {
        throw std::runtime_error("could not extract shader description");
    }

    reflectionData.version = shaderDesc.Version;
    reflectionData.generatorName = shaderDesc.Creator;

    reflectionData.boundResources.reserve(shaderDesc.BoundResources);

    // reflectionData.constantBufferCount = shaderDesc.ConstantBuffers;
    // reflectionData.inputParameterCount = shaderDesc.InputParameters;
    // reflectionData.outputParameterCount = shaderDesc.OutputParameters;

    for (auto idx = UINT{ 0 }, count = shaderDesc.BoundResources; idx < count; idx++) {
        auto bindingDesc = D3D12_SHADER_INPUT_BIND_DESC{};
        if (auto result = dxcShaderReflection->GetResourceBindingDesc(idx, &bindingDesc); !SUCCEEDED(result)) {
            throw std::runtime_error("could not extract bind description");
        }

        auto& boundResource = reflectionData.boundResources.emplace_back();
        boundResource.name = bindingDesc.Name;
        boundResource.space = bindingDesc.Space;
        boundResource.bindPoint = bindingDesc.BindPoint;
        boundResource.bindCount = bindingDesc.BindCount;

        // todo:: convert enums
        // boundResource.textureComponetType = bindingDesc.ReturnType;
    }
}
}