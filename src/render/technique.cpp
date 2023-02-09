//
// Created by anton on 2/8/23.
//

#include "technique.h"
#include "resources/resourceManager.h"
#include "vulkan/shaderModule.h"

namespace cyclonite::render {
using namespace easy_mp;

namespace {
auto getVulkanRasterizationShaderStage(ShaderStage stage) -> VkShaderStageFlagBits
{
    auto flags = VkShaderStageFlagBits{ VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM };

    switch (stage) {
        case ShaderStage::VERTEX_STAGE:
            flags = VK_SHADER_STAGE_VERTEX_BIT;
            break;
        case ShaderStage::TESSELATION_CONTROL_STAGE:
            flags = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
            break;
        case ShaderStage::TESSELATION_EVALUATION_STAGE:
            flags = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
            break;
        case ShaderStage::GEOMETRY_STAGE:
            flags = VK_SHADER_STAGE_GEOMETRY_BIT;
            break;
        case ShaderStage::FRAGMENT_STAGE:
            flags = VK_SHADER_STAGE_FRAGMENT_BIT;
            break;
        default:
            throw std::runtime_error("unexpected shader stage");
    }

    assert(flags != VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM);
    return flags;
}

auto getVulkanPolygonMode(Technique::PolygonMode polygonMode) -> VkPolygonMode
{
    auto mode = VkPolygonMode{ VK_POLYGON_MODE_MAX_ENUM };

    mode = (polygonMode == Technique::PolygonMode::FILL)
             ? VK_POLYGON_MODE_FILL
             : (polygonMode == Technique::PolygonMode::LINE)
                 ? VK_POLYGON_MODE_LINE
                 : (polygonMode == Technique::PolygonMode::POINT) ? VK_POLYGON_MODE_POINT : VK_POLYGON_MODE_MAX_ENUM;

    assert(mode != VK_POLYGON_MODE_MAX_ENUM);
    return mode;
}

auto getVulkanCullFaceMode(Technique::CullFace cullFaceMode) -> VkCullModeFlagBits
{
    auto mode = VkCullModeFlagBits{ VK_CULL_MODE_NONE };

    mode = (cullFaceMode == Technique::CullFace::BACK)
             ? VK_CULL_MODE_BACK_BIT
             : (cullFaceMode == Technique::CullFace::FRONT)
                 ? VK_CULL_MODE_FRONT_BIT
                 : (cullFaceMode == Technique::CullFace::BOTH) ? VK_CULL_MODE_FRONT_AND_BACK : VK_CULL_MODE_NONE;

    return mode;
}

auto getVulkanSampleCount(uint32_t sampleCount) -> VkSampleCountFlagBits
{
    auto samples = VkSampleCountFlagBits{ VK_SAMPLE_COUNT_1_BIT };

    if (samples & VK_SAMPLE_COUNT_64_BIT)
        samples = VK_SAMPLE_COUNT_64_BIT;
    else if (samples & VK_SAMPLE_COUNT_32_BIT)
        samples = VK_SAMPLE_COUNT_32_BIT;
    else if (samples & VK_SAMPLE_COUNT_16_BIT)
        samples = VK_SAMPLE_COUNT_16_BIT;
    else if (samples & VK_SAMPLE_COUNT_8_BIT)
        samples = VK_SAMPLE_COUNT_8_BIT;
    else if (samples & VK_SAMPLE_COUNT_4_BIT)
        samples = VK_SAMPLE_COUNT_4_BIT;
    else if (samples & VK_SAMPLE_COUNT_2_BIT)
        samples = VK_SAMPLE_COUNT_2_BIT;

    return samples;
}
}

resources::Resource::ResourceTag Technique::tag{};

Technique::Technique()
  : Resource{}
  , alphaModePerOutput_{}
  , colorBlendEquationPerOutput_{}
  , alphaBlendEquationPerOutput_{}
  , colorSrcBlendFactorPerOutput_{}
  , colorDstBlendFactorPerOutput_{}
  , alphaSrcBlendFactorPerOutput_{}
  , alphaDstBlendFactorPerOutput_{}
  , writeMaskPerOutput_{}
  , cullFace_{ CullFace::BACK }
  , polygonMode_{ PolygonMode::FILL }
  , alphaCutoff_{ 0.f }
  , lineWith_{ 1.f }
  , depthBiasClamp_{ 0.f }
  , depthBiasConstantFactor_{ 0.f }
  , depthBiasSlopeFactor_{ 0.f }
  , shaders_{}
  , shaderFlags_{}
  , pipeline_{}
  , isExpired_{ false }
{
    std::fill(alphaModePerOutput_.begin(), alphaModePerOutput_.end(), AlphaMode::OPAQUE);
    std::fill(colorBlendEquationPerOutput_.begin(), colorBlendEquationPerOutput_.end(), BlendEquation::FUNC_ADD);
    std::fill(alphaBlendEquationPerOutput_.begin(), alphaBlendEquationPerOutput_.end(), BlendEquation::FUNC_ADD);
    std::fill(colorSrcBlendFactorPerOutput_.begin(), colorSrcBlendFactorPerOutput_.end(), BlendFactor::ONE);
    std::fill(colorDstBlendFactorPerOutput_.begin(), colorDstBlendFactorPerOutput_.end(), BlendFactor::ZERO);
    std::fill(alphaSrcBlendFactorPerOutput_.begin(), alphaSrcBlendFactorPerOutput_.end(), BlendFactor::ONE);
    std::fill(alphaDstBlendFactorPerOutput_.begin(), alphaDstBlendFactorPerOutput_.end(), BlendFactor::ZERO);
    std::fill(writeMaskPerOutput_.begin(),
              writeMaskPerOutput_.end(),
              output_write_mask_t{ value_cast(OutputChannel::R) | value_cast(OutputChannel::G) |
                                   value_cast(OutputChannel::B) | value_cast(OutputChannel::A) });

    flags_.set(value_cast(Flags::DEPTH_WRITE), true);
    flags_.set(value_cast(Flags::DEPTH_TEST), true);
    flags_.set(value_cast(Flags::RASTERIZER_DISCARD_ENABLE), false);
    flags_.set(value_cast(Flags::DEPTH_CLAMP_ENABLE), false);
    flags_.set(value_cast(Flags::DEPTH_BIAS_ENABLE), false);

    std::fill(shaders_.begin(), shaders_.end(), resources::Resource::Id{ std::numeric_limits<uint64_t>::max() });
}

void Technique::update(resources::ResourceManager const& resourceManager,
                       bool sampleShadingEnabled /* = false*/,
                       uint32_t sampleCount /* = 1*/)
{
    if (!isExpired_ && pipeline_)
        return;

    if (pipeline_)
        pipeline_.reset();

    auto stages = { ShaderStage::VERTEX_STAGE,
                    ShaderStage::TESSELATION_CONTROL_STAGE,
                    ShaderStage::TESSELATION_EVALUATION_STAGE,
                    ShaderStage::GEOMETRY_STAGE,
                    ShaderStage::FRAGMENT_STAGE };

    assert(rasterization_shader_stage_count_v == std::size(stages));

    auto shaderStageCount = uint32_t{ 0 };
    auto shaderStages = std::array<VkPipelineShaderStageCreateInfo, rasterization_shader_stage_count_v>{};

    for (auto stage : stages) {
        if (!shaderFlags_.test(shaderStageCount))
            continue;

        auto& sm = resourceManager.get(shaders_[shaderStageCount]).template as<vulkan::ShaderModule>();

        auto& sci = shaderStages[shaderStageCount++];
        sci.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        sci.stage = getVulkanRasterizationShaderStage(stage);
        sci.module = sm.handle();
        sci.pName = sm.entryPointName().data();
    }

    // TODO:: extend reflection to extract vertex shader input as well
    auto vertexInput = VkPipelineVertexInputStateCreateInfo{};
    vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInput.vertexBindingDescriptionCount = 0;
    vertexInput.vertexAttributeDescriptionCount = 0;

    auto assemblyState = VkPipelineInputAssemblyStateCreateInfo{};
    assemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    assemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    assemblyState.primitiveRestartEnable = VK_FALSE; // TODO:: make it comes from material

    // TODO:: adds viewport struct and rasterization workspace

    auto rasterizationState = VkPipelineRasterizationStateCreateInfo{};
    rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationState.depthClampEnable = VkBool32{ flags_.test(value_cast(Flags::DEPTH_CLAMP_ENABLE)) };
    rasterizationState.rasterizerDiscardEnable = VkBool32{ flags_.test(value_cast(Flags::RASTERIZER_DISCARD_ENABLE)) };
    rasterizationState.polygonMode = getVulkanPolygonMode(polygonMode_);
    rasterizationState.lineWidth = lineWith_;
    rasterizationState.cullMode = getVulkanCullFaceMode(cullFace_);
    rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizationState.depthBiasEnable = VkBool32{ flags_.test(value_cast(Flags::DEPTH_BIAS_ENABLE)) };
    rasterizationState.depthBiasClamp = depthBiasClamp_;
    rasterizationState.depthBiasConstantFactor = depthBiasConstantFactor_;
    rasterizationState.depthBiasSlopeFactor = depthBiasSlopeFactor_;

    auto multisampleState = VkPipelineMultisampleStateCreateInfo{};
    multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleState.sampleShadingEnable = VkBool32{ sampleShadingEnabled };
    multisampleState.rasterizationSamples = getVulkanSampleCount(sampleCount);

    // TODO::
}
}