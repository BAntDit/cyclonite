//
// Created by anton on 11/2/25.
//

#include "vkPipeline.h"
#include "gfx/device.h"
#include "gfx/pipelineBindingSchema.h"
#include "gfx/renderPass.h"
#include "gfx/shader.h"
#include "internal/utils.h"
#include "vkException.h"
#include <bit>

#if defined(GFX_DRIVER_VULKAN)
namespace cyclonite::gfx::vulkan {
Pipeline::Pipeline(core::ResourceManagerBase* resourceManager,
                   core::ResourceId resourceId,
                   core::ResourceSharedRef deviceRef,
                   PipelineType type,
                   PipelineCreationFlagBits creationFlags,
                   core::ResourceSharedRef bindingSchemaRef,
                   PrimitiveTopology primitiveTopology,
                   bool primitiveRestartEnable,
                   core::ResourceSharedRef renderPassRef,
                   RasterizationState const& rasterizationState,
                   std::span<core::ResourceSharedRef const> shaders)
  : core::ResourceBase{ resourceManager, resourceId, false }
  , shaders_{}
  , bindingSchemaRef_{ std::move(bindingSchemaRef) }
  , renderPassRef_{ std::move(renderPassRef) }
  , vkPipeline_{ deviceRef.as<type_traits::platform_implementation_t<gfx::Device>>().handle(), vkDestroyPipeline }
  , type_{ type }
{
    if (type == PipelineType::PrimitiveRasterization) {
        initPrimitiveRasterizationPipeline(
          deviceRef, creationFlags, primitiveTopology, primitiveRestartEnable, rasterizationState, shaders);
    } else if (type == PipelineType::Compute) {
        assert(shaders.size() == 1);
        initComputePipeline(deviceRef, creationFlags, shaders[0]);
    } else if (type == PipelineType::GeometricShading) {
        // TODO:: 
    } else if (type == PipelineType::RayTracing) {
        // TODO:: 
    }
}

Pipeline::Pipeline(core::ResourceManagerBase* resourceManager,
                   core::ResourceId resourceId,
                   core::ResourceSharedRef deviceRef,
                   PipelineCreationFlagBits creationFlags,
                   core::ResourceSharedRef bindingSchemaRef,
                   PrimitiveTopology primitiveTopology,
                   bool primitiveRestartEnable,
                   core::ResourceSharedRef renderPassRef,
                   RasterizationState const& rasterizationState,
                   std::span<core::ResourceSharedRef const> shaders)
  : Pipeline{ resourceManager,
              resourceId,
              std::move(deviceRef),
              PipelineType::PrimitiveRasterization,
              creationFlags,
              std::move(bindingSchemaRef),
              primitiveTopology,
              primitiveRestartEnable,
              std::move(renderPassRef),
              rasterizationState,
              shaders }
{
}

Pipeline::Pipeline(core::ResourceManagerBase* resourceManager,
                   core::ResourceId resourceId,
                   core::ResourceSharedRef deviceRef,
                   PipelineCreationFlagBits creationFlags,
                   core::ResourceSharedRef bindingSchemaRef,
                   core::ResourceSharedRef shaderRef)
  : Pipeline{ resourceManager,
              resourceId,
              std::move(deviceRef),
              PipelineType::Compute,
              creationFlags,
              std::move(bindingSchemaRef),
              PrimitiveTopology::LINE_LIST,
              false,
              core::ResourceSharedRef{},
              RasterizationState{},
              std::array<core::ResourceSharedRef, 1>{ std::move(shaderRef) }

  }
{
}

void Pipeline::initPrimitiveRasterizationPipeline(core::ResourceSharedRef const& deviceRef,
                                                  PipelineCreationFlagBits creationFlags,
                                                  PrimitiveTopology primitiveTopology,
                                                  bool primitiveRestartEnable,
                                                  RasterizationState const& rasterizationState,
                                                  std::span<core::ResourceSharedRef const> shaders)
{
    assert(deviceRef.valid());
    auto const& device = deviceRef.as<type_traits::platform_implementation_t<gfx::Device>>();

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
    auto& renderPass = renderPassRef_.as<type_traits::platform_implementation_t<gfx::RenderPass>>();

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

    auto pipelineRasterizationState = VkPipelineRasterizationStateCreateInfo{};
    pipelineRasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    pipelineRasterizationState.depthClampEnable =
      rasterizationState.flags.test(RasterizationStateFlags::DEPTH_CLAMP_ENABLE);
    pipelineRasterizationState.rasterizerDiscardEnable =
      rasterizationState.flags.test(RasterizationStateFlags::RASTERIZER_DISCARD_ENABLE);
    pipelineRasterizationState.polygonMode = internal::getPolygonMode(rasterizationState.polygonMode);
    pipelineRasterizationState.cullMode = internal::getCullMode(rasterizationState.cullMode);
    pipelineRasterizationState.frontFace = internal::getFrontFace(rasterizationState.frontFace);
    pipelineRasterizationState.depthBiasEnable =
      rasterizationState.flags.test(RasterizationStateFlags::DEPTH_BIAS_ENABLE);
    pipelineRasterizationState.depthBiasConstantFactor = rasterizationState.depthBiasConstantFactor;
    pipelineRasterizationState.depthBiasClamp = rasterizationState.depthBiasClamp;
    pipelineRasterizationState.depthBiasSlopeFactor = rasterizationState.depthBiasSlopeFactor;
    pipelineRasterizationState.lineWidth = rasterizationState.lineWidth;

    auto multisampleState = VkPipelineMultisampleStateCreateInfo{};
    multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleState.sampleShadingEnable = VK_FALSE; // not going to support
    multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    auto depthStencilState = VkPipelineDepthStencilStateCreateInfo{};
    depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilState.depthTestEnable = rasterizationState.flags.test(RasterizationStateFlags::DEPTH_TEST_ENABLE);
    depthStencilState.depthWriteEnable = rasterizationState.flags.test(RasterizationStateFlags::DEPTH_WRITE_ENABLE);
    depthStencilState.depthCompareOp = internal::getCompareOp(rasterizationState.depthComparison);
    depthStencilState.depthBoundsTestEnable =
      rasterizationState.flags.test(RasterizationStateFlags::DEPTH_BOUNDS_TEST_ENABLE);
    depthStencilState.stencilTestEnable = rasterizationState.flags.test(RasterizationStateFlags::STENCIL_TEST_ENABLE);
    depthStencilState.front.failOp = internal::getStencilOp(rasterizationState.frontStencilState.failOp);
    depthStencilState.front.depthFailOp = internal::getStencilOp(rasterizationState.frontStencilState.depthFailOp);
    depthStencilState.front.passOp = internal::getStencilOp(rasterizationState.frontStencilState.passOp);
    depthStencilState.front.compareOp = internal::getCompareOp(rasterizationState.frontStencilState.compareOp);
    depthStencilState.front.compareMask = rasterizationState.frontStencilState.compareMask;
    depthStencilState.front.writeMask = rasterizationState.frontStencilState.writeMask;
    depthStencilState.front.reference = rasterizationState.frontStencilState.reference;
    depthStencilState.back.failOp = internal::getStencilOp(rasterizationState.backStencilState.failOp);
    depthStencilState.back.depthFailOp = internal::getStencilOp(rasterizationState.backStencilState.depthFailOp);
    depthStencilState.back.passOp = internal::getStencilOp(rasterizationState.backStencilState.passOp);
    depthStencilState.back.compareOp = internal::getCompareOp(rasterizationState.backStencilState.compareOp);
    depthStencilState.back.compareMask = rasterizationState.backStencilState.compareMask;
    depthStencilState.back.writeMask = rasterizationState.backStencilState.writeMask;
    depthStencilState.back.reference = rasterizationState.backStencilState.reference;
    depthStencilState.maxDepthBounds = 1.0f;
    depthStencilState.minDepthBounds = 0.0f;

    // TODO:: blending support
    auto blendEnabled = rasterizationState.flags.test(RasterizationStateFlags::BLEND_ENABLE);

    auto attachmentBlendStates =
      std::array<VkPipelineColorBlendAttachmentState, config_t::max_color_attachment_count_v>{};
    auto actualAttachmentCount = renderPass.colorAttachmentCount();
    for (auto i = size_t{ 0 }, count = static_cast<size_t>(actualAttachmentCount); i < count; i++) {
        auto& attachmentBlendState = attachmentBlendStates[i];
        attachmentBlendState.blendEnable = blendEnabled;
        attachmentBlendState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        attachmentBlendState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        attachmentBlendState.colorBlendOp = VK_BLEND_OP_ADD;
        attachmentBlendState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        attachmentBlendState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        attachmentBlendState.alphaBlendOp = VK_BLEND_OP_ADD;
        attachmentBlendState.colorWriteMask =
          VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    }

    auto colorBlendState = VkPipelineColorBlendStateCreateInfo{};
    colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendState.logicOpEnable = rasterizationState.flags.test(RasterizationStateFlags::BLEND_LOGICAL_OP_ENABLE);
    colorBlendState.logicOp = VK_LOGIC_OP_AND;
    colorBlendState.attachmentCount = actualAttachmentCount;
    colorBlendState.pAttachments = attachmentBlendStates.data();

    assert(bindingSchemaRef_.valid());
    auto& bindingSchema = bindingSchemaRef_.as<type_traits::platform_implementation_t<gfx::PipelineBindingSchema>>();

    auto pipelineInfo = VkGraphicsPipelineCreateInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.flags = creationFlags.cast_to<VkPipelineCreateFlags>();
    pipelineInfo.stageCount = std::popcount(shaderStageBits.value);
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.pVertexInputState = &vertexInfo;
    pipelineInfo.pInputAssemblyState = &assemblyState;
    pipelineInfo.pTessellationState = &tesselationState;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &pipelineRasterizationState;
    pipelineInfo.pMultisampleState = &multisampleState;
    pipelineInfo.pDepthStencilState = &depthStencilState;
    pipelineInfo.pColorBlendState = &colorBlendState;
    pipelineInfo.layout = bindingSchema.handle();
    pipelineInfo.renderPass = renderPass.handle();

    if (auto vkResult =
          vkCreateGraphicsPipelines(device.handle(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &vkPipeline_);
        vkResult != VK_SUCCESS) {
        throw Exception{ vkResult, "vkCreateGraphicsPipelines" };
    }
}

void Pipeline::initComputePipeline(core::ResourceSharedRef const& deviceRef,
                                   PipelineCreationFlagBits creationFlags,
                                   core::ResourceSharedRef shaderRef)
{
    assert(deviceRef.valid());
    auto const& device = deviceRef.as<type_traits::platform_implementation_t<gfx::Device>>();

    assert(shaderRef.valid());
    auto const& shader = shaderRef.as<type_traits::platform_implementation_t<gfx::Shader>>();

    auto shaderStageInfo = VkPipelineShaderStageCreateInfo{};
    shaderStageInfo.flags = shader.creationFlags().cast_to<VkPipelineShaderStageCreateFlags>();
    shaderStageInfo.stage = shader.vulkanStage();
    shaderStageInfo.module = shader.handle();
    shaderStageInfo.pName = shader.entryPointName().data();
    shaders_.add(shaderRef, shader.stage());

    assert(bindingSchemaRef_.valid());
    auto& bindingSchema = bindingSchemaRef_.as<type_traits::platform_implementation_t<gfx::PipelineBindingSchema>>();

    auto pipelineCreateInfo = VkComputePipelineCreateInfo{};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.flags = creationFlags.cast_to<VkPipelineCreateFlags>();
    pipelineCreateInfo.stage = shaderStageInfo;
    pipelineCreateInfo.layout = bindingSchema.handle();

    if (auto vkResult =
          vkCreateComputePipelines(device.handle(), VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &vkPipeline_);
        vkResult != VK_SUCCESS) {
        throw Exception{ vkResult, "vkCreateComputePipelines" };
    }
}
}
#endif // GFX_DRIVER_VULKAN
