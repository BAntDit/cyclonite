//
// Created by anton on 11/2/25.
//

#include "vkPipeline.h"
#include "gfx/device.h"
#include "gfx/shader.h"
#include "internal/utils.h"
#include <bit>

#if defined(GFX_DRIVER_VULKAN)
// TODO:: move all shaders, reflection and root signture (pipeline layout) into one class (Shader Set)

namespace cyclonite::gfx::vulkan {
Pipeline::Pipeline(core::ResourceManagerBase* resourceManager,
                   core::ResourceId resourceId,
                   core::ResourceSharedRef deviceRef,
                   PipelineType type,
                   PipelineCreationFlagBits creationFlags,
                   PrimitiveTopology primitiveTopology,
                   bool primitiveRestartEnable,
                   std::array<core::ResourceSharedRef, config_t::max_shader_stage_count_v> const& shaders)
  : core::ResourceBase{ resourceManager, resourceId, false }
  , vkPipeline_{ deviceRef.as<type_traits::platform_implementation_t<gfx::Device>>().handle(), vkDestroyPipeline }
{
    if (type == PipelineType::Graphics) {
        initGraphicsPipeline(creationFlags, primitiveTopology, primitiveRestartEnable, shaders);
    } else if (type == PipelineType::Compute) {
        // TODO:: ...
    }
}

void Pipeline::initGraphicsPipeline(
  PipelineCreationFlagBits creationFlags,
  PrimitiveTopology primitiveTopology,
  bool primitiveRestartEnable,
  std::array<core::ResourceSharedRef, config_t::max_shader_stage_count_v> const& shaders)
{
    // shader stages:
    auto shaderStageBits = ShaderStageFlagBits{};
    auto shaderStages = std::array<VkPipelineShaderStageCreateInfo, config_t::max_shader_stage_count_v>{};

    for (auto const& shaderRef : shaders) {
        if (shaderRef.valid()) {
            auto const& shader = shaderRef.as<type_traits::platform_implementation_t<gfx::Shader>>();

            auto stageIndex = std::popcount(shaderStageBits.value);
            auto& stageInfo = shaderStages[stageIndex];

            assert(!shaderStageBits.test(shader.stage()));
            shaderStageBits.set(shader.stage());

            stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            stageInfo.flags = shader.creationFlags().cast_to<VkPipelineShaderStageCreateFlags>();
            stageInfo.stage = shader.vulkanStage();
            stageInfo.module = shader.handle();
            stageInfo.pName = shader.entryPointName().data();
        }
    }

    // vertex input (empty, because no vertex fetching, but programable vertex pulling only)
    auto vertexInfo = VkPipelineVertexInputStateCreateInfo{};
    vertexInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    // assembly state
    auto assemblyState = VkPipelineInputAssemblyStateCreateInfo{};
    assemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    assemblyState.topology = internal::getPrimitiveTopology(primitiveTopology);
    assemblyState.primitiveRestartEnable = VkBool32{ primitiveRestartEnable };

    // tesselation state (not supported yet) // TODO:: 
    auto tesselationState = VkPipelineTessellationStateCreateInfo{};
    tesselationState.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;

    auto pipelineInfo = VkGraphicsPipelineCreateInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.flags = creationFlags.cast_to<VkPipelineCreateFlags>();
    pipelineInfo.stageCount = std::popcount(shaderStageBits.value);
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.pVertexInputState = &vertexInfo;
    pipelineInfo.pInputAssemblyState = &assemblyState;
    pipelineInfo.pTessellationState = &tesselationState;
}
}
#endif // GFX_DRIVER_VULKAN
