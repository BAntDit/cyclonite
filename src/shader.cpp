//
// Created by anton on 3/30/26.
//

#include "shader.h"
#include "deserialization.h"
#include "shaderModuleBinary.h"

namespace cyclonite {
Shader::Shader(core::ResourceManagerBase* resourceManager,
               core::ResourceId resourceId,
               std::string_view name,
               boost::uuids::uuid const& uuid)
  : core::ResourceBase{ resourceManager, resourceId, false }
  , resources::ManagedResource<cyclonite::Shader>{ name, uuid }
  , rawData_{}
  , hwShader_{}
  , bindings_{}
{
}

namespace {
auto getShaderStage(uint32_t profileCode) -> gfx::ShaderStageFlags
{
    gfx::ShaderStageFlags stage = gfx::ShaderStageFlags::STAGE_COUNT;

    if (profileCode <= 9) {
        stage = gfx::ShaderStageFlags::FRAGMENT;
    } else if (profileCode >= 10 && profileCode <= 19) {
        stage = gfx::ShaderStageFlags::VERTEX;
    } else if (profileCode >= 20 && profileCode <= 29) {
        stage = gfx::ShaderStageFlags::GEOMETRY;
    } else if (profileCode >= 30 && profileCode <= 39) {
        stage = gfx::ShaderStageFlags::TESSELLATION_CONTROL;
    } else if (profileCode >= 40 && profileCode <= 49) {
        stage = gfx::ShaderStageFlags::TESSELLATION_EVALUATION;
    } else if (profileCode >= 50 && profileCode <= 59) {
        stage = gfx::ShaderStageFlags::COMPUTE;
    } else if (profileCode >= 69 && profileCode <= 78) {
        stage = gfx::ShaderStageFlags::MESH;
    }

    assert(stage != gfx::ShaderStageFlags::STAGE_COUNT);
    return stage;
}
}

void Shader::loadImpl(std::istream& stream)
{
    [[maybe_unused]] auto magicNumber = uint32_t{ 0 };
    stream.read(reinterpret_cast<char*>(&magicNumber), sizeof(uint32_t));
    assert(magicNumber == shared::SHADER_MODULE_MAGIC_NUMBER);

    auto smBlockHeaders = std::vector<shared::ShaderModuleBlockHeader>{};
    shared::readStream(smBlockHeaders, stream);

    auto moduleInfo = shared::ShaderInfoBlock{};
    auto code = std::vector<uint32_t>{};
    auto reflectionData = shared::ShaderReflectionData{};

    for (auto blockHeader : smBlockHeaders) {
        auto [baseOffset, blockOffset, size, id] = blockHeader;
        stream.seekg(baseOffset + blockOffset, std::ios::beg);

        switch (id) {
            case shared::SHADER_MODULE_INFO_BLOCK: {
                shared::readStream(moduleInfo, stream);
            } break;
            case shared::SHADER_MODULE_SPIRV_BLOCK: {
                shared::readStream(code, stream);
            } break;
            case shared::SHADER_MODULE_REFLECTION_BLOCK: {
                shared::readStream(reflectionData, stream);
            } break;
            default:
                assert(false);
        }
    }

    rawData_ = std::make_unique<raw_data_t>();
    rawData_->entryName = moduleInfo.entryPoint;
    rawData_->stage = getShaderStage(moduleInfo.targetProfile);
    std::swap(rawData_->code, code);

    // std::swap(code_, code);

    // TODO:: fill bindings
}
}
