//
// Created by anton on 11/2/25.
//

#include "vkPipeline.h"
#include "gfx/device.h"
#include "gfx/renderPass.h"
#include "gfx/shader.h"
#include "internal/utils.h"
#include <bit>

#if defined(GFX_DRIVER_VULKAN)
// TODO:: move all shaders, reflection and root signture (pipeline layout) into one class (Shader Set)

namespace cyclonite::gfx::vulkan {
Pipeline::Pipeline(
  core::ResourceManagerBase* resourceManager,
  core::ResourceId resourceId,
  core::ResourceSharedRef deviceRef,
  PipelineType type,
  PipelineCreationFlagBits creationFlags,
  PrimitiveTopology primitiveTopology,
  bool primitiveRestartEnable,
  core::ResourceSharedRef renderPassRef,
  std::array<core::ResourceSharedRef, metrix::value_cast(ShaderStageFlags::STAGE_COUNT)> const& shaders)
  : core::ResourceBase{ resourceManager, resourceId, false }
  , shaders_{}
  , renderPassRef_{ std::move(renderPassRef) }
  , vkPipeline_{ deviceRef.as<type_traits::platform_implementation_t<gfx::Device>>().handle(), vkDestroyPipeline }
{
    if (type == PipelineType::PrimitiveRasterization) {
        initPrimitiveRasterizationPipeline(creationFlags, primitiveTopology, primitiveRestartEnable, shaders);
    } else if (type == PipelineType::Compute) {
        // TODO:: ...
    }
}

void Pipeline::initPrimitiveRasterizationPipeline(
  PipelineCreationFlagBits creationFlags,
  PrimitiveTopology primitiveTopology,
  bool primitiveRestartEnable,
  std::array<core::ResourceSharedRef, metrix::value_cast(ShaderStageFlags::STAGE_COUNT)> const& shaders)
{
    // shader stages:
    auto shaderStageBits = ShaderStageFlagBits{};
    auto shaderStages =
      std::array<VkPipelineShaderStageCreateInfo, metrix::value_cast(ShaderStageFlags::STAGE_COUNT)>{};

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

            shaders_.add(shaderRef, shader.stage());
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

    assert(renderPassRef_.valid());
    auto& renderPass = renderPassRef_.as<gfx::RenderPass>();

    auto viewport = VkViewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(renderPass.width());
    viewport.height = static_cast<float>(renderPass.height());
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    auto scissor = VkRect2D{};
    scissor.offset = { 0, 0 };
    scissor.extent = { renderPass.width(), renderPass.height() };

    auto viewportState = VkPipelineViewportStateCreateInfo{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    auto pipelineInfo = VkGraphicsPipelineCreateInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.flags = creationFlags.cast_to<VkPipelineCreateFlags>();
    pipelineInfo.stageCount = std::popcount(shaderStageBits.value);
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.pVertexInputState = &vertexInfo;
    pipelineInfo.pInputAssemblyState = &assemblyState;
    pipelineInfo.pTessellationState = &tesselationState;
    pipelineInfo.pViewportState = &viewportState;
}
}
#endif // GFX_DRIVER_VULKAN
