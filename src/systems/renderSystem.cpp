//
// Created by bantdit on 3/22/20.
//

#include "renderSystem.h"
#include "vulkan/shaderModule.h"

std::vector<uint32_t> const defaultVertexShaderCode = {
#include "shaders/default.vert.spv.cpp.txt"
};

std::vector<uint32_t> const defaultFragmentShaderCode = {
#include "shaders/default.frag.spv.cpp.txt"
};

namespace cyclonite::systems {
void RenderSystem::_createDummyPipeline(vulkan::Device& device,
                                        VkRenderPass renderPass,
                                        std::array<uint32_t, 4> const& viewport,
                                        bool depthStencilRequired)
{
    // DUMMY PIPELINE, just for now
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
    vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInput.vertexBindingDescriptionCount = 0;
    vertexInput.vertexAttributeDescriptionCount = 0;

    VkPipelineInputAssemblyStateCreateInfo assemblyState = {};
    assemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    assemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    assemblyState.primitiveRestartEnable = VK_FALSE;

    auto&& [x, y, width, height] = viewport;

    VkViewport vkViewport = {};
    vkViewport.x = x;
    vkViewport.y = y;
    vkViewport.width = width;
    vkViewport.height = height;
    vkViewport.minDepth = 0.0;
    vkViewport.maxDepth = 1.0;

    VkRect2D vkScissor = {};
    vkScissor.offset.x = x;
    vkScissor.offset.y = y;
    vkScissor.extent.width = width;
    vkScissor.extent.height = height;

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &vkViewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &vkScissor;

    VkPipelineRasterizationStateCreateInfo rasterizationState = {};
    rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationState.depthClampEnable = VK_FALSE;
    rasterizationState.rasterizerDiscardEnable = VK_FALSE; // just for now
    rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationState.lineWidth = 1.0f;
    rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizationState.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampleState = {};
    multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleState.sampleShadingEnable = VK_FALSE;
    multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; // TODO:: must come from render target

    // TMP::
    // TODO:: must come from RT
    VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState = {};
    pipelineColorBlendAttachmentState.blendEnable = VK_FALSE;
    pipelineColorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    pipelineColorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    pipelineColorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
    pipelineColorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    pipelineColorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    pipelineColorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
    pipelineColorBlendAttachmentState.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo colorBlendState = {}; // TODO:: must come from render target
    colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendState.logicOpEnable = VK_FALSE;
    colorBlendState.logicOp = VK_LOGIC_OP_COPY;
    colorBlendState.attachmentCount = 1;
    colorBlendState.pAttachments = &pipelineColorBlendAttachmentState;
    colorBlendState.blendConstants[0] = 0.0f;
    colorBlendState.blendConstants[1] = 0.0f;
    colorBlendState.blendConstants[2] = 0.0f;
    colorBlendState.blendConstants[3] = 0.0f;

    std::array<VkDescriptorSetLayoutBinding, 3> bindings = { VkDescriptorSetLayoutBinding{},
                                                             VkDescriptorSetLayoutBinding{},
                                                             VkDescriptorSetLayoutBinding{} };

    bindings[0].binding = 0;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    bindings[1].binding = 1;
    bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    bindings[1].descriptorCount = 1;
    bindings[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    bindings[2].binding = 2;
    bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bindings[2].descriptorCount = 1;
    bindings[2].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutCreateInfo descriptorSetLayout = {};
    descriptorSetLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayout.bindingCount = static_cast<uint32_t>(bindings.size());
    descriptorSetLayout.pBindings = bindings.data();

    if (auto result =
          vkCreateDescriptorSetLayout(device.handle(), &descriptorSetLayout, nullptr, &descriptorSetLayout_);
        result != VK_SUCCESS) {
        throw std::runtime_error("could not create descriptor set layout");
    }

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = 1;
    pipelineLayoutCreateInfo.pSetLayouts = &std::as_const(descriptorSetLayout_);

    if (auto result = vkCreatePipelineLayout(device.handle(), &pipelineLayoutCreateInfo, nullptr, &pipelineLayout_);
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
    graphicsPipelineCreateInfo.layout = static_cast<VkPipelineLayout>(pipelineLayout_);
    graphicsPipelineCreateInfo.renderPass = renderPass;
    graphicsPipelineCreateInfo.subpass = 0;
    graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;

    if (depthStencilRequired) {
        VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo = {};

        depthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
        depthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
        depthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
        depthStencilStateCreateInfo.minDepthBounds = 0.0f;
        depthStencilStateCreateInfo.maxDepthBounds = 1.0f;
        depthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;
        depthStencilStateCreateInfo.front = {};
        depthStencilStateCreateInfo.back = {};

        graphicsPipelineCreateInfo.pDepthStencilState = &depthStencilStateCreateInfo;
    }

    if (auto result = vkCreateGraphicsPipelines(
          device.handle(), VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, &pipeline_);
        result != VK_SUCCESS) {
        throw std::runtime_error("could not create graphics pipeline");
    }
}

void RenderSystem::_createDummyDescriptorPool(vulkan::Device& device, size_t maxSets)
{
    std::array<VkDescriptorPoolSize, 3> poolSizes = { VkDescriptorPoolSize{},
                                                      VkDescriptorPoolSize{},
                                                      VkDescriptorPoolSize{} };

    poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[0].descriptorCount = maxSets;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[1].descriptorCount = maxSets;
    poolSizes[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[2].descriptorCount = maxSets;

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
    descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCreateInfo.maxSets = static_cast<uint32_t>(maxSets);
    descriptorPoolCreateInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    descriptorPoolCreateInfo.pPoolSizes = poolSizes.data();

    if (auto result = vkCreateDescriptorPool(device.handle(), &descriptorPoolCreateInfo, nullptr, &descriptorPool_);
        result != VK_SUCCESS) {
        throw std::runtime_error("could not create descriptor pool");
    }
}

void RenderSystem::finish()
{
    auto gfxStop = taskManager_->submit([&, this]() -> void {
        if (auto result = vkQueueWaitIdle(device_->graphicsQueue()); result != VK_SUCCESS) {
            if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
                throw std::runtime_error("could not wait until graphics queue gets idle, out of host memory");
            }

            if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
                throw std::runtime_error("could not wait until graphics queue gets idle, out of device memory");
            }

            if (result == VK_ERROR_DEVICE_LOST) {
                throw std::runtime_error("could not wait until graphics queue gets idle, device lost");
            }
        }
    });

    auto transferStop = taskManager_->submit([&, this]() -> void {
        if (auto result = vkQueueWaitIdle(device_->hostTransferQueue()); result != VK_SUCCESS) {
            if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
                throw std::runtime_error("could not wait until transfer queue gets idle, out of host memory");
            }

            if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
                throw std::runtime_error("could not wait until transfer queue gets idle, out of device memory");
            }

            if (result == VK_ERROR_DEVICE_LOST) {
                throw std::runtime_error("could not wait until transfer queue gets idle, device lost");
            }
        }
    });

    transferStop.get();
    gfxStop.get();
}
}