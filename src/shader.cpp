//
// Created by anton on 3/30/26.
//

#include "shader.h"
#include "binaryStreamReader.h"
#include "gfx/device.h"
#include "serialization.h"
#include "shaderModuleBinary.h"

namespace cyclonite {
Shader::Shader(core::ResourceManagerBase* resourceManager,
               core::ResourceId resourceId,
               resources::ResourceGroupBase* resourceGroup,
               std::string_view name,
               boost::uuids::uuid const& uuid)
  : core::ResourceBase{ resourceManager, resourceId, false }
  , resources::ManagedResource<cyclonite::Shader>{ resourceGroup, name, uuid }
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

auto getDescriptorType(shared::ShaderResourceType resType,
                       shared::ResourceViewDimension dimension,
                       bool isDynamic) -> gfx::DescriptorType
{
    auto descType = gfx::DescriptorType::DESCRIPTOR_TYPE_COUNT;

    if (resType == shared::ShaderResourceType::Sampler) {
        descType = gfx::DescriptorType::SAMPLER;
    } else if (resType == shared::ShaderResourceType::Texture) {
        if (dimension == shared::ResourceViewDimension::Texture1D ||
            dimension == shared::ResourceViewDimension::Texture1DArray ||
            dimension == shared::ResourceViewDimension::Texture2D ||
            dimension == shared::ResourceViewDimension::Texture2DArray ||
            dimension == shared::ResourceViewDimension::Texture2DMS ||
            dimension == shared::ResourceViewDimension::Texture2DMSArray ||
            dimension == shared::ResourceViewDimension::TextureCube ||
            dimension == shared::ResourceViewDimension::TextureCubeArray ||
            dimension == shared::ResourceViewDimension::Texture3D) {
            descType = gfx::DescriptorType::SAMPLED_IMAGE; // TODO:: test if shader has sampler
        } else if (dimension == shared::ResourceViewDimension::Buffer) {
            descType = gfx::DescriptorType::UNIFORM_TEXEL_BUFFER;
        }
    } else if (resType == shared::ShaderResourceType::RwTyped) {
        if (dimension == shared::ResourceViewDimension::Buffer) {
            descType = gfx::DescriptorType::STORAGE_TEXEL_BUFFER;
        } else {
            descType = gfx::DescriptorType::SAMPLED_IMAGE;
        }
    } else if (resType == shared::ShaderResourceType::CBuffer) {
        descType = isDynamic ? gfx::DescriptorType::UNIFORM_BUFFER_DYNAMIC : gfx::DescriptorType::UNIFORM_BUFFER;
    } else if (resType == shared::ShaderResourceType::StructuredBuffer &&
               dimension == shared::ResourceViewDimension::Buffer) {
        descType = gfx::DescriptorType::STORAGE_BUFFER;
    } else if (resType == shared::ShaderResourceType::RwStructuredBuffer &&
               dimension == shared::ResourceViewDimension::Buffer) {
        descType = gfx::DescriptorType::STORAGE_BUFFER;
    } else if (resType == shared::ShaderResourceType::AppendStructuredBuffer ||
               resType == shared::ShaderResourceType::ConsumeStructuredBuffer ||
               resType == shared::ShaderResourceType::RwStructuredWithCounter) {
        descType = gfx::DescriptorType::STORAGE_BUFFER;
    }

    return descType;
}
}

void Shader::loadImpl(std::istream& stream)
{
    auto shaderModuleBinary = shared::ShaderModuleBinary{};

    auto headersDeserializer =
      shared::Deserializer{ shared::makeAccessChain<&shared::ShaderModuleBinary::testMagicNumber>(),
                            shared::makeAccessChain<&shared::ShaderModuleBinary::blockHeaders,
                                                    &shared::ShaderModuleBlockHeader::setBlockHeaderData>() };

    auto reader = shared::BinaryStreamReader{ stream, shared::Endian::Little };

    headersDeserializer.deserialize(shaderModuleBinary, reader);

    for (auto const& header : shaderModuleBinary.blockHeaders) {
        auto [baseOffset, blockOffset, size, id] = header;

        reader.setStreamOffset(baseOffset + blockOffset);

        switch (id) {
            case shared::SHADER_MODULE_INFO_BLOCK: {
                auto infoBlockDeserializaer = shared::Deserializer{
                    shared::makeAccessChain<&shared::ShaderModuleBinary::infoBlock,
                                            &shared::ShaderInfoBlock::entryPoint>(),
                    shared::makeAccessChain<&shared::ShaderModuleBinary::infoBlock,
                                            &shared::ShaderInfoBlock::targetProfile>(),
                    shared::makeAccessChain<&shared::ShaderModuleBinary::infoBlock, &shared::ShaderInfoBlock::name>(),
                    shared::makeAccessChain<&shared::ShaderModuleBinary::infoBlock, &shared::ShaderInfoBlock::uuid>()
                };
                infoBlockDeserializaer.deserialize(shaderModuleBinary, reader);
            } break;
            case shared::SHADER_MODULE_SPIRV_BLOCK: {
                auto spirvDeserializaer =
                  shared::Deserializer{ shared::makeAccessChain<&shared::ShaderModuleBinary::spirvCode>() };
                spirvDeserializaer.deserialize(shaderModuleBinary, reader);
            } break;
            case shared::SHADER_MODULE_REFLECTION_BLOCK: {
                auto reflectionDeserializaer =
                  shared::Deserializer{ shared::makeAccessChain<&shared::ShaderModuleBinary::reflectionData,
                                                                &shared::ShaderReflectionData::version>(),
                                        shared::makeAccessChain<&shared::ShaderModuleBinary::reflectionData,
                                                                &shared::ShaderReflectionData::generatorName>(),
                                        shared::makeAccessChain<&shared::ShaderModuleBinary::reflectionData,
                                                                &shared::ShaderReflectionData::boundResources,
                                                                &shared::BoundResource::setResourceData>(),
                                        shared::makeAccessChain<&shared::ShaderModuleBinary::reflectionData,
                                                                &shared::ShaderReflectionData::constantBuffers,
                                                                &shared::ConstantBufferReflection::setBufferData>() };

                reflectionDeserializaer.deserialize(shaderModuleBinary, reader);
            } break;
            default:
                assert(false);
        }
    }
    rawData_ = std::make_unique<raw_data_t>();
    rawData_->entryName = shaderModuleBinary.infoBlock.entryPoint;
    rawData_->stage = getShaderStage(shaderModuleBinary.infoBlock.targetProfile);
    std::swap(rawData_->code, shaderModuleBinary.spirvCode);

    bindings_.reserve(shaderModuleBinary.reflectionData.boundResources.size());
    for (auto&& [name, type, space, point, count, texComponentCount, sampleCount, dimension] :
         shaderModuleBinary.reflectionData.boundResources) {

        auto descriptorType =
          getDescriptorType(type, dimension, space == metrix::value_cast(gfx::DescriptorSpace::PER_BATCH_DYNAMIC));
        auto stageFlags = gfx::ShaderStageFlagBits{ rawData_->stage };
        auto descriptorSetLayoutFlags = gfx::DescriptorSetLayoutFlagBits{};
        auto bindingFlags = gfx::BindingFlagBits{};

        descriptorSetLayoutFlags.set(gfx::DescriptorSetLayoutFlags::UPDATE_AFTER_BIND);
        bindingFlags.set(gfx::BindingFlags::UPDATE_AFTER_BIND);

        bindings_.emplace_back(space, point, descriptorType, count, stageFlags, descriptorSetLayoutFlags, bindingFlags);
    }
}

void Shader::prepareImpl()
{
    auto& g = group();
    auto ref = g.deviceRef();
    auto& device = ref.as<gfx::Device>();

    assert(rawData_);
    auto& binaryCode = rawData_->code;
    auto creationFlags = gfx::ShaderStageCreationFlagBits{};
    auto stage = rawData_->stage;
    auto ep = std::string_view{ rawData_->entryName };

    hwShader_ = device.createShader(binaryCode.size(), binaryCode.data(), creationFlags, stage, ep);
    assert(hwShader_.valid());

    rawData_.reset();
}
}
