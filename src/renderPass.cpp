//
// Created by bantdit on 11/21/19.
//

#include "renderPass.h"
#include "vulkan/shaderModule.h"

std::vector<uint32_t> const defaultVertexShaderCode = {
#include "shaders/default.frag.spv.cpp.txt"
};

std::vector<uint32_t> const defaultFragmentShaderCode = {
#include "shaders/default.frag.spv.cpp.txt"
};

namespace cyclonite {
auto RenderPass::begin(vulkan::Device const& device) -> VkFence
{
    auto currentCainIndex = renderTarget_->currentChainIndex();

    vkWaitForFences(device.handle(), 1, &frameFences_[currentCainIndex], VK_TRUE, std::numeric_limits<uint64_t>::max());

    auto nextChainIndex = renderTarget_->getNextChainIndex(device);

    if (renderTargetFences_[nextChainIndex] != VK_NULL_HANDLE) {
        vkWaitForFences(
          device.handle(), 1, &renderTargetFences_[nextChainIndex], VK_TRUE, std::numeric_limits<uint64_t>::max());
    }

    return renderTargetFences_[nextChainIndex] = static_cast<VkFence>(frameFences_[currentCainIndex]);
}

void RenderPass::_createDummyPipeline(vulkan::Device& device)
{
    vulkan::ShaderModule vertexShaderModule{ device, defaultVertexShaderCode, VK_SHADER_STAGE_VERTEX_BIT };
    vulkan::ShaderModule fragmentShaderModule{ device, defaultFragmentShaderCode, VK_SHADER_STAGE_FRAGMENT_BIT };

    VkPipelineShaderStageCreateInfo vertexShaderStageInfo = {};
    vertexShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexShaderStageInfo.module = vertexShaderModule.handle();
    vertexShaderStageInfo.pName = u8"main";

    VkPipelineShaderStageCreateInfo fragmentShaderStageInfo = {};
    fragmentShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentShaderStageInfo.module = fragmentShaderModule.handle();
    fragmentShaderStageInfo.pName = u8"main";

    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages = { vertexShaderStageInfo, fragmentShaderStageInfo };

    VkPipelineVertexInputStateCreateInfo vertexInput = {};
    vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexInput.vertexBindingDescriptionCount = 0;
    vertexInput.vertexAttributeDescriptionCount = 0;

    VkPipelineInputAssemblyStateCreateInfo assemblyState = {};
    assemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    assemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    assemblyState.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = renderTarget_->width();
    viewport.height = renderTarget_->height();
    viewport.minDepth = 0.0;
    viewport.maxDepth = 1.0;

    VkRect2D scissor = {};
    scissor.offset = { 0, 0 };
    scissor.extent = { renderTarget_->width(), renderTarget_->height() };

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizationState = {};
    rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationState.depthClampEnable = VK_FALSE;
    rasterizationState.rasterizerDiscardEnable = VK_FALSE;
    rasterizationState.rasterizerDiscardEnable = VK_FALSE; // just for now
    rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationState.lineWidth = 1.0f;
    rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizationState.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampleState = {};
    multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleState.sampleShadingEnable = VK_FALSE;
    multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; // TODO:: must come from render target

    std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachmentStates = {};

    colorBlendAttachmentStates.reserve(renderTarget_->colorAttachmentCount());

    // TODO:: must come as structures array from renderTarget
    for (size_t i = 0, count = renderTarget_->colorAttachmentCount(); i < count; i++) {
        colorBlendAttachmentStates.emplace_back(VkPipelineColorBlendAttachmentState{
          VK_FALSE,
          VK_BLEND_FACTOR_ONE,
          VK_BLEND_FACTOR_ZERO,
          VK_BLEND_OP_ADD,
          VK_BLEND_FACTOR_ONE,
          VK_BLEND_FACTOR_ZERO,
          VK_BLEND_OP_ADD,
          VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT });
    }

    VkPipelineColorBlendStateCreateInfo colorBlendState = {};
    colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendState.logicOpEnable = VK_FALSE;
    colorBlendState.logicOp = VK_LOGIC_OP_COPY;
    colorBlendState.attachmentCount = colorBlendAttachmentStates.size();
    colorBlendState.pAttachments = colorBlendAttachmentStates.data();
    colorBlendState.blendConstants[0] = 0.0f;
    colorBlendState.blendConstants[1] = 0.0f;
    colorBlendState.blendConstants[2] = 0.0f;
    colorBlendState.blendConstants[3] = 0.0f;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pushConstantRangeCount = 0;

    if (auto result = vkCreatePipelineLayout(device.handle(), &pipelineLayoutInfo, nullptr, &vkDummyPipelineLayout_);
        result != VK_SUCCESS) {
        throw std::runtime_error("could not create graphics pipeline layout");
    }

    VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {};
    graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    graphicsPipelineCreateInfo.stageCount = shaderStages.size();
    graphicsPipelineCreateInfo.pStages = shaderStages.data();
    graphicsPipelineCreateInfo.pVertexInputState = &vertexInput;
    graphicsPipelineCreateInfo.pInputAssemblyState = &assemblyState;
    graphicsPipelineCreateInfo.pViewportState = &viewportState;
    graphicsPipelineCreateInfo.pRasterizationState = &rasterizationState;
    graphicsPipelineCreateInfo.pMultisampleState = &multisampleState;
    graphicsPipelineCreateInfo.pColorBlendState = &colorBlendState;
    graphicsPipelineCreateInfo.layout = static_cast<VkPipelineLayout>(vkDummyPipelineLayout_);
    graphicsPipelineCreateInfo.renderPass = static_cast<VkRenderPass>(vkRenderPass_);
    graphicsPipelineCreateInfo.subpass = 0;
    graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;

    if (auto result = vkCreateGraphicsPipelines(
          device.handle(), VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, &vkDummyPipeline_);
        result != VK_SUCCESS) {
        throw std::runtime_error("could not create graphics pipeline");
    }
}
}
