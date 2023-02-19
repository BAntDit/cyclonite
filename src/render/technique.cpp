//
// Created by anton on 2/8/23.
//

#include "technique.h"
#include "baseRenderTarget.h"
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

auto getVulkanBlendFactor(Technique::BlendFactor blendFactor) -> VkBlendFactor
{
    auto r = VkBlendFactor{ VK_BLEND_FACTOR_MAX_ENUM };

    switch (blendFactor) {
        case Technique::BlendFactor::ZERO:
            r = VK_BLEND_FACTOR_ZERO;
            break;
        case Technique::BlendFactor::ONE:
            r = VK_BLEND_FACTOR_ONE;
            break;
        case Technique::BlendFactor::SRC_COLOR:
            r = VK_BLEND_FACTOR_SRC_COLOR;
            break;
        case Technique::BlendFactor::ONE_MINUS_SRC_COLOR:
            r = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
            break;
        case Technique::BlendFactor::DST_COLOR:
            r = VK_BLEND_FACTOR_DST_COLOR;
            break;
        case Technique::BlendFactor::ONE_MINUS_DST_COLOR:
            r = VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
            break;
        case Technique::BlendFactor::SRC_ALPHA:
            r = VK_BLEND_FACTOR_SRC_ALPHA;
            break;
        case Technique::BlendFactor::ONE_MINUS_SRC_ALPHA:
            r = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            break;
        case Technique::BlendFactor::DST_ALPHA:
            r = VK_BLEND_FACTOR_DST_ALPHA;
            break;
        case Technique::BlendFactor::ONE_MINUS_DST_ALPHA:
            r = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
            break;
        case Technique::BlendFactor::CONSTANT_COLOR:
            r = VK_BLEND_FACTOR_CONSTANT_COLOR;
            break;
        case Technique::BlendFactor::ONE_MINUS_CONSTANT_COLOR:
            r = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
            break;
        case Technique::BlendFactor::CONSTANT_ALPHA:
            r = VK_BLEND_FACTOR_CONSTANT_ALPHA;
            break;
        case Technique::BlendFactor::ONE_MINUS_CONSTANT_ALPHA:
            r = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
            break;
        case Technique::BlendFactor::SRC_ALPHA_SATURATE:
            r = VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
            break;
        case Technique::BlendFactor::SRC1_COLOR:
            r = VK_BLEND_FACTOR_SRC1_COLOR;
            break;
        case Technique::BlendFactor::ONE_MINUS_SRC1_COLOR:
            r = VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
            break;
        case Technique::BlendFactor::SRC1_ALPHA:
            r = VK_BLEND_FACTOR_SRC1_ALPHA;
            break;
        case Technique::BlendFactor::ONE_MINUS_SRC1_ALPHA:
            r = VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
            break;
        default:
            throw std::runtime_error("unexpected blend factor value");
    }

    assert(r != VK_BLEND_FACTOR_MAX_ENUM);
    return r;
}

auto getVulkanBlendOp(Technique::BlendEquation blendEquation) -> VkBlendOp
{
    auto r = VkBlendOp{ VK_BLEND_OP_MAX_ENUM };

    r = (blendEquation == Technique::BlendEquation::FUNC_ADD)
          ? VK_BLEND_OP_ADD
          : (blendEquation == Technique::BlendEquation::FUNC_SUBTRACT)
              ? VK_BLEND_OP_SUBTRACT
              : (blendEquation == Technique::BlendEquation::FUNC_REVERSE_SUBTRACT) ? VK_BLEND_OP_REVERSE_SUBTRACT
                                                                                   : VK_BLEND_OP_MAX_ENUM;

    assert(r != VK_BLEND_OP_MAX_ENUM);
    return r;
}
}

Technique::Technique()
  : stageMask_{}
  , shaderModuleCount_{ 0 }
  , shaderModules_{}
  , entryPoints_{}
  , colorOutputCount_{ 0 }
  , alphaModePerOutput_{}
  , colorBlendEquationPerOutput_{}
  , alphaBlendEquationPerOutput_{}
  , colorSrcBlendFactorPerOutput_{}
  , colorDstBlendFactorPerOutput_{}
  , alphaSrcBlendFactorPerOutput_{}
  , alphaDstBlendFactorPerOutput_{}
  , writeMaskPerOutput_{}
  , blendConstants_{ 1.f }
  , alphaCutoff_{ 0.f }
  , cullFace_{ CullFace::BACK }
  , polygonMode_{ PolygonMode::FILL }
  , lineWith_{ 1.f }
  , depthBiasClamp_{ 0.f }
  , depthBiasConstantFactor_{ 0.f }
  , depthBiasSlopeFactor_{ 0.f }
  , flags_{}
  , descriptorSetLayoutCount_{ 0 }
  , descriptorSetLayouts_{}
  , pipeline_{}
  , name_{}
  , isExpired_{ true }
{
    std::fill(shaderModules_.begin(), shaderModules_.end(), resources::Resource::Id{});

    std::fill(entryPoints_.begin(),
              entryPoints_.end(),
              shader_entry_point_t{
                .name = "", .stage = ShaderStage::COUNT, .moduleIndex = std::numeric_limits<size_t>::max() });

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
}

void Technique::update(resources::ResourceManager const& resourceManager,
                       BaseRenderTarget const& rt,
                       bool multisampleShadingEnabled /* = false*/,
                       uint32_t sampleCount /* = 1*/,
                       bool forceUpdate /* = false*/)
{
    isExpired_ |= forceUpdate;

    // TODO:: handle render target resize
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

    VkViewport vkViewport = {};
    vkViewport.x = static_cast<real>(0);
    vkViewport.y = static_cast<real>(0);
    vkViewport.width = static_cast<real>(rt.width());
    vkViewport.height = static_cast<real>(rt.height());
    vkViewport.minDepth = 0.0;
    vkViewport.maxDepth = 1.0;

    VkRect2D vkScissor = {};
    vkScissor.offset.x = static_cast<int32_t>(0);
    vkScissor.offset.y = static_cast<int32_t>(0);
    vkScissor.extent.width = rt.width();
    vkScissor.extent.height = rt.height();

    auto viewportState = VkPipelineViewportStateCreateInfo{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &vkViewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &vkScissor;

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
    multisampleState.sampleShadingEnable = VkBool32{ multisampleShadingEnabled };
    multisampleState.rasterizationSamples = getVulkanSampleCount(sampleCount);

    auto pipelineColorBlendAttachmentStates =
      std::array<VkPipelineColorBlendAttachmentState, render_target_output_semantic_count_v>{};

    auto attachmentCount = rt.colorAttachmentCount();
    assert(attachmentCount <= render_target_output_semantic_count_v);

    for (auto i = size_t{ 0 }; i < attachmentCount; i++) {
        auto& pipelineColorBlendAttachmentState = pipelineColorBlendAttachmentStates[i];

        auto alphaMode = alphaModePerOutput_[i];
        auto colorBlendSrcFactor = colorSrcBlendFactorPerOutput_[i];
        auto colorBlendDstFactor = colorDstBlendFactorPerOutput_[i];
        auto alphaBlendSrcFactor = alphaSrcBlendFactorPerOutput_[i];
        auto alphaBlendDstFactor = alphaDstBlendFactorPerOutput_[i];

        auto colorBlendOp = colorBlendEquationPerOutput_[i];
        auto alphaBlendOp = alphaBlendEquationPerOutput_[i];

        pipelineColorBlendAttachmentState.blendEnable =
          VkBool32{ alphaMode == AlphaMode::BLEND || alphaMode == AlphaMode::ADDITIVE_BLEND ||
                    alphaMode == AlphaMode::SUBTRACTIVE_BLEND || alphaMode == AlphaMode::SUBTRACTIVE_BLEND ||
                    alphaMode == AlphaMode::CUSTOM_BLEND };
        pipelineColorBlendAttachmentState.srcColorBlendFactor = getVulkanBlendFactor(colorBlendSrcFactor);
        pipelineColorBlendAttachmentState.dstColorBlendFactor = getVulkanBlendFactor(colorBlendDstFactor);
        pipelineColorBlendAttachmentState.colorBlendOp = getVulkanBlendOp(colorBlendOp);
        pipelineColorBlendAttachmentState.srcAlphaBlendFactor = getVulkanBlendFactor(alphaBlendSrcFactor);
        pipelineColorBlendAttachmentState.dstAlphaBlendFactor = getVulkanBlendFactor(alphaBlendDstFactor);
        pipelineColorBlendAttachmentState.alphaBlendOp = getVulkanBlendOp(alphaBlendOp);

        auto writeMask = writeMaskPerOutput_[i];
        pipelineColorBlendAttachmentState.colorWriteMask = static_cast<VkColorComponentFlags>(writeMask.to_ulong());

        auto colorBlendState = VkPipelineColorBlendStateCreateInfo{};
        colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlendState.logicOpEnable = VK_FALSE; // ???
        colorBlendState.logicOp = VK_LOGIC_OP_COPY;
        colorBlendState.attachmentCount = static_cast<uint32_t>(attachmentCount);
        colorBlendState.pAttachments = pipelineColorBlendAttachmentStates.data();
        colorBlendState.blendConstants[0] = blendConstants_.r;
        colorBlendState.blendConstants[1] = blendConstants_.g;
        colorBlendState.blendConstants[2] = blendConstants_.b;
        colorBlendState.blendConstants[3] = blendConstants_.a;
    }

    isExpired_ = false;
}
}