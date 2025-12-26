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
#include <cassert>

namespace cyclonite::tools {
namespace {
auto getTextureReturnComponentType(D3D_RESOURCE_RETURN_TYPE returnType) -> shared::TextureResourceComponetType
{
    auto result = shared::TextureResourceComponetType::Undefined;

    switch (returnType) {
        case D3D_RETURN_TYPE_UNORM:
            result = shared::TextureResourceComponetType::UNORM;
            break;
        case D3D_RETURN_TYPE_SNORM:
            result = shared::TextureResourceComponetType::SNORM;
            break;
        case D3D_RETURN_TYPE_SINT:
            result = shared::TextureResourceComponetType::SINT;
            break;
        case D3D_RETURN_TYPE_UINT:
            result = shared::TextureResourceComponetType::UINT;
            break;
        case D3D_RETURN_TYPE_FLOAT:
            result = shared::TextureResourceComponetType::FLOAT;
            break;
        case D3D_RETURN_TYPE_MIXED:
            result = shared::TextureResourceComponetType::MIXED;
            break;
        case D3D_RETURN_TYPE_CONTINUED:
            result = shared::TextureResourceComponetType::CONTINUED;
            break;
        case D3D_RETURN_TYPE_DOUBLE:
            result = shared::TextureResourceComponetType::DOUBLE;
            break;
        default:
            assert(false);
    }

    return result;
}

auto getResourceType(D3D_SHADER_INPUT_TYPE inputType) -> shared::ShaderResourceType
{
    auto result = shared::ShaderResourceType::Undefined;

    switch (inputType) {
        case D3D_SIT_CBUFFER:
            result = shared::ShaderResourceType::CBuffer;
            break;
        case D3D_SIT_TBUFFER:
            result = shared::ShaderResourceType::TBuffer;
            break;
        case D3D_SIT_TEXTURE:
            result = shared::ShaderResourceType::Texture;
            break;
        case D3D_SIT_SAMPLER:
            result = shared::ShaderResourceType::Sampler;
            break;
        case D3D_SIT_UAV_RWTYPED:
            result = shared::ShaderResourceType::RwTyped;
            break;
        case D3D_SIT_STRUCTURED:
            result = shared::ShaderResourceType::StructuredBuffer;
            break;
        case D3D_SIT_UAV_RWSTRUCTURED:
            result = shared::ShaderResourceType::RwStructuredBuffer;
            break;
        case D3D_SIT_BYTEADDRESS:
            result = shared::ShaderResourceType::ByteAddress;
            break;
        case D3D_SIT_UAV_RWBYTEADDRESS:
            result = shared::ShaderResourceType::RwByteAddress;
            break;
        case D3D_SIT_UAV_APPEND_STRUCTURED:
            result = shared::ShaderResourceType::AppendStructuredBuffer;
            break;
        case D3D_SIT_UAV_CONSUME_STRUCTURED:
            result = shared::ShaderResourceType::ConsumeStructuredBuffer;
            break;
        case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
            result = shared::ShaderResourceType::RwStructuredWithCounter;
            break;
        case D3D_SIT_RTACCELERATIONSTRUCTURE:
            result = shared::ShaderResourceType::RtAccelerationStructure;
            break;
        case D3D_SIT_UAV_FEEDBACKTEXTURE:
            result = shared::ShaderResourceType::FeedbackTexture;
            break;
        default:
            assert(false);
    }

    return result;
}

auto getResourceViewDimension(D3D_SRV_DIMENSION dimension) -> shared::ResourceViewDimension
{
    auto result = shared::ResourceViewDimension::Undefined;

    switch (dimension) {
        case D3D_SRV_DIMENSION_UNKNOWN:
            result = shared::ResourceViewDimension::Undefined;
            break;
        case D3D_SRV_DIMENSION_BUFFER:
            result = shared::ResourceViewDimension::Buffer;
            break;
        case D3D_SRV_DIMENSION_TEXTURE1D:
            result = shared::ResourceViewDimension::Texture1D;
            break;
        case D3D_SRV_DIMENSION_TEXTURE1DARRAY:
            result = shared::ResourceViewDimension::Texture1DArray;
            break;
        case D3D_SRV_DIMENSION_TEXTURE2D:
            result = shared::ResourceViewDimension::Texture2D;
            break;
        case D3D_SRV_DIMENSION_TEXTURE2DARRAY:
            result = shared::ResourceViewDimension::Texture2DArray;
            break;
        case D3D_SRV_DIMENSION_TEXTURE2DMS:
            result = shared::ResourceViewDimension::Texture2DMS;
            break;
        case D3D_SRV_DIMENSION_TEXTURE2DMSARRAY:
            result = shared::ResourceViewDimension::Texture2DMSArray;
            break;
        case D3D_SRV_DIMENSION_TEXTURE3D:
            result = shared::ResourceViewDimension::Texture3D;
            break;
        case D3D_SRV_DIMENSION_TEXTURECUBE:
            result = shared::ResourceViewDimension::TextureCube;
            break;
        case D3D_SRV_DIMENSION_TEXTURECUBEARRAY:
            result = shared::ResourceViewDimension::TextureCubeArray;
            break;
        case D3D_SRV_DIMENSION_BUFFEREX:
            result = shared::ResourceViewDimension::BufferEx;
            break;
        default:
            assert(false);
    }

    return result;
}

auto getConstantBufferType(D3D_CBUFFER_TYPE cbType) -> shared::ConstantBufferType
{
    auto result = shared::ConstantBufferType::Undefined;

    switch (cbType) {
        case D3D_CT_CBUFFER:
            result = shared::ConstantBufferType::CBuffer;
            break;
        case D3D_CT_TBUFFER:
            result = shared::ConstantBufferType::TBuffer;
            break;
        case D3D_CT_INTERFACE_POINTERS:
            result = shared::ConstantBufferType::InterfacePointer;
            break;
        case D3D_CT_RESOURCE_BIND_INFO:
            result = shared::ConstantBufferType::ResourceBindInfo;
            break;
        assert(false);
    }

    return result;
}
}

void DxReflection::getShaderDesc(ID3D12ShaderReflection* dxcShaderReflection)
{
    auto shaderDesc = D3D12_SHADER_DESC{};
    if (auto result = dxcShaderReflection->GetDesc(&shaderDesc); !SUCCEEDED(result)) {
        throw std::runtime_error("could not extract shader description");
    }

    reflectionData.version = shaderDesc.Version;
    reflectionData.generatorName = shaderDesc.Creator;

    reflectionData.boundResources.reserve(shaderDesc.BoundResources);
    for (auto idx = UINT{ 0 }, count = shaderDesc.BoundResources; idx < count; idx++) {
        auto bindingDesc = D3D12_SHADER_INPUT_BIND_DESC{};
        if (auto result = dxcShaderReflection->GetResourceBindingDesc(idx, &bindingDesc); !SUCCEEDED(result)) {
            throw std::runtime_error("could not extract bind description");
        }

        auto& boundResource = reflectionData.boundResources.emplace_back();
        boundResource.name = bindingDesc.Name;
        boundResource.type = getResourceType(bindingDesc.Type);
        boundResource.space = bindingDesc.Space;
        boundResource.bindPoint = bindingDesc.BindPoint;
        boundResource.bindCount = bindingDesc.BindCount;
        boundResource.textureComponetType = getTextureReturnComponentType(bindingDesc.ReturnType);
        boundResource.sampleCount = bindingDesc.NumSamples;
        boundResource.dimension = getResourceViewDimension(bindingDesc.Dimension);
    }

    reflectionData.constantBuffers.reserve(shaderDesc.ConstantBuffers);
    for (auto idx = UINT{ 0 }, count = shaderDesc.ConstantBuffers; idx < count; idx++) {
        auto bufferDesc = D3D12_SHADER_BUFFER_DESC{};
        auto* constantBufferReflection = dxcShaderReflection->GetConstantBufferByIndex(idx);

        if (auto result = constantBufferReflection->GetDesc(&bufferDesc); !SUCCEEDED(result)) {
            throw std::runtime_error("could not extract constant buffer description");
        }

        auto& constantBuffer = reflectionData.constantBuffers.emplace_back();
        constantBuffer.name = bufferDesc.Name;
        constantBuffer.type = getConstantBufferType(bufferDesc.Type);
        constantBuffer.size = bufferDesc.Size;

        constantBuffer.variables.reserve(bufferDesc.Variables);
        for (auto vidx = UINT{ 0 }, vcount = bufferDesc.Variables; vidx < vcount; vidx++) {
            auto* cbVariableRefl = constantBufferReflection->GetVariableByIndex(vidx);

            // cbVariableRefl->GetDesc();
        }
    }
}
}