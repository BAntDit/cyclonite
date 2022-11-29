//
// Created by anton on 3/21/21.
//

#include "graphicsNode.h"
#include "shaders/shaderDefinitions.h"

namespace cyclonite::compositor {
void createPass(vulkan::Device& device,
                uint32_t subPassIndex,
                bool depthStencilRequired,
                VkRenderPass renderPass,
                std::array<uint32_t, 4> const& viewport,
                uint32_t commandBufferCount,
                uint32_t imageInputCount,
                uint32_t attachmentCount,
                PassType inPassType,
                PassType& outPassType,
                std::unique_ptr<vulkan::ShaderModule>& vertexSceneShader,
                std::unique_ptr<vulkan::ShaderModule>& fragmentSceneShader,
                std::unique_ptr<vulkan::ShaderModule>& vertexScreenShader,
                std::unique_ptr<vulkan::ShaderModule>& fragmentScreenShader,
                vulkan::Handle<VkDescriptorPool>& outDescriptorPool,
                vulkan::Handle<VkDescriptorSetLayout>& outDescriptorSetLayout,
                vulkan::Handle<VkPipelineLayout>& outPipelineLayout,
                vulkan::Handle<VkPipeline>& outPipeline)
{
    // pass type
    outPassType = inPassType;

    constexpr auto maxDescriptorCount = size_t{ 35 };

    const auto bufferDescriptorCount = inPassType == PassType::SCENE ? uint32_t{ 3 } : uint32_t{ 0 };
    const auto descriptorCount = bufferDescriptorCount + imageInputCount;

    // descriptor pool
    {
        outDescriptorPool = vulkan::Handle<VkDescriptorPool>{ device.handle(), vkDestroyDescriptorPool };

        const auto maxSets = commandBufferCount;

        std::array<VkDescriptorPoolSize, maxDescriptorCount> poolSizes = {};

        if (inPassType == PassType::SCENE) {
            poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            poolSizes[0].descriptorCount = maxSets;
            poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            poolSizes[1].descriptorCount = maxSets;
            poolSizes[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            poolSizes[2].descriptorCount = maxSets;

        } else if (inPassType == PassType::SCREEN) {
            // poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            // poolSizes[0].descriptorCount = maxSets;
        } else {
            assert(false);
        }

        {
            auto poolSize = VkDescriptorPoolSize{};
            poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            poolSize.descriptorCount = maxSets;

            std::fill_n(std::next(poolSizes.begin(), bufferDescriptorCount), imageInputCount, poolSize);
        }

        auto descriptorPoolCreateInfo = VkDescriptorPoolCreateInfo{};
        descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorPoolCreateInfo.maxSets = static_cast<uint32_t>(maxSets);
        descriptorPoolCreateInfo.poolSizeCount = descriptorCount;
        descriptorPoolCreateInfo.pPoolSizes = poolSizes.data();

        if (auto result =
              vkCreateDescriptorPool(device.handle(), &descriptorPoolCreateInfo, nullptr, &outDescriptorPool);
            result != VK_SUCCESS) {
            throw std::runtime_error("could not create descriptor pool");
        }
    }

    // DUMMY PIPELINE, just for now
    {
        if (inPassType == PassType::SCENE) {
            if (!vertexSceneShader)
                vertexSceneShader = std::make_unique<vulkan::ShaderModule>(
                  device,
                  shaders::getShader(shaders::ShaderType::DEFAULT_VERTEX_TRANSFORM_SHADER),
                  VK_SHADER_STAGE_VERTEX_BIT);

            if (!fragmentSceneShader)
                fragmentSceneShader = std::make_unique<vulkan::ShaderModule>(
                  device,
                  shaders::getShader(shaders::ShaderType::DEFAULT_G_BUFFER_FRAGMENT_SHADER),
                  VK_SHADER_STAGE_FRAGMENT_BIT);
        } else if (inPassType == PassType::SCREEN) {
            if (!vertexScreenShader)
                vertexScreenShader = std::make_unique<vulkan::ShaderModule>(
                  device,
                  shaders::getShader(shaders::ShaderType::SCREEN_TRIANGLE_VERTEX_SHADER),
                  VK_SHADER_STAGE_VERTEX_BIT);

            if (!fragmentScreenShader)
                fragmentScreenShader = std::make_unique<vulkan::ShaderModule>(
                  device,
                  shaders::getShader(shaders::ShaderType::SCREEN_G_BUFFER_DEBUG_FRAGMENT_SHADER),
                  VK_SHADER_STAGE_FRAGMENT_BIT);
        }

        VkPipelineShaderStageCreateInfo vertexShaderStageInfo = {};
        vertexShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertexShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertexShaderStageInfo.module =
          (inPassType == PassType::SCENE) ? vertexSceneShader->handle() : vertexScreenShader->handle();
        vertexShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragmentShaderStageInfo = {};
        fragmentShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragmentShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragmentShaderStageInfo.module =
          (inPassType == PassType::SCENE) ? fragmentSceneShader->handle() : fragmentScreenShader->handle();
        fragmentShaderStageInfo.pName = "main";

        std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages = { vertexShaderStageInfo,
                                                                        fragmentShaderStageInfo };

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
        vkViewport.x = static_cast<real>(x);
        vkViewport.y = static_cast<real>(y);
        vkViewport.width = static_cast<real>(width);
        vkViewport.height = static_cast<real>(height);
        vkViewport.minDepth = 0.0;
        vkViewport.maxDepth = 1.0;

        VkRect2D vkScissor = {};
        vkScissor.offset.x = static_cast<int32_t>(x);
        vkScissor.offset.y = static_cast<int32_t>(y);
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
        rasterizationState.cullMode = inPassType == PassType::SCREEN ? VK_CULL_MODE_FRONT_BIT : VK_CULL_MODE_BACK_BIT;
        rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizationState.depthBiasEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampleState = {};
        multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampleState.sampleShadingEnable = VK_FALSE;
        multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; // TODO:: must come from render target

        // TMP::
        // TODO:: must come from material
        constexpr size_t maxColorAttachmentCount = 32;
        auto pipelineColorBlendAttachmentStates =
          std::array<VkPipelineColorBlendAttachmentState, maxColorAttachmentCount>{};

        assert(attachmentCount < maxColorAttachmentCount);
        for (uint32_t i = 0; i < attachmentCount; i++) {
            auto& pipelineColorBlendAttachmentState = pipelineColorBlendAttachmentStates[i];

            pipelineColorBlendAttachmentState.blendEnable = VK_FALSE;
            pipelineColorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
            pipelineColorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
            pipelineColorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
            pipelineColorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            pipelineColorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
            pipelineColorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
            pipelineColorBlendAttachmentState.colorWriteMask =
              VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        }

        VkPipelineColorBlendStateCreateInfo colorBlendState = {}; // TODO:: and blend state too
        colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlendState.logicOpEnable = VK_FALSE;
        colorBlendState.logicOp = VK_LOGIC_OP_COPY;
        colorBlendState.attachmentCount = attachmentCount;
        colorBlendState.pAttachments = pipelineColorBlendAttachmentStates.data();
        colorBlendState.blendConstants[0] = 0.0f;
        colorBlendState.blendConstants[1] = 0.0f;
        colorBlendState.blendConstants[2] = 0.0f;
        colorBlendState.blendConstants[3] = 0.0f;

        std::array<VkDescriptorSetLayoutBinding, maxDescriptorCount> bindings = {};

        if (inPassType == PassType::SCENE) {
            bindings[0] = VkDescriptorSetLayoutBinding{};
            bindings[0].binding = 0;
            bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            bindings[0].descriptorCount = 1;
            bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
            bindings[1] = VkDescriptorSetLayoutBinding{};
            bindings[1].binding = 1;
            bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            bindings[1].descriptorCount = 1;
            bindings[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
            bindings[2] = VkDescriptorSetLayoutBinding{};
            bindings[2].binding = 2;
            bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            bindings[2].descriptorCount = 1;
            bindings[2].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        } else if (inPassType == PassType::SCREEN) {
            // bindings[0].binding = 0;
            // bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            // bindings[0].descriptorCount = 1;
            // bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        } else {
            assert(false);
        }

        {
            for (auto i = uint32_t{ bufferDescriptorCount }; i < descriptorCount; i++) {
                bindings[i] = VkDescriptorSetLayoutBinding{};
                bindings[i].binding = i;
                bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                bindings[i].descriptorCount = 1;
                bindings[i].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            }
        }

        auto descriptorSetLayout = VkDescriptorSetLayoutCreateInfo{};
        descriptorSetLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayout.bindingCount = static_cast<uint32_t>(descriptorCount);
        descriptorSetLayout.pBindings = bindings.data();

        outDescriptorSetLayout = vulkan::Handle<VkDescriptorSetLayout>{ device.handle(), vkDestroyDescriptorSetLayout };

        if (auto result =
              vkCreateDescriptorSetLayout(device.handle(), &descriptorSetLayout, nullptr, &outDescriptorSetLayout);
            result != VK_SUCCESS) {
            throw std::runtime_error("could not create descriptor set layout");
        }

        auto pipelineLayoutCreateInfo = VkPipelineLayoutCreateInfo{};
        pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCreateInfo.setLayoutCount = 1;
        pipelineLayoutCreateInfo.pSetLayouts = &std::as_const(outDescriptorSetLayout);

        outPipelineLayout = vulkan::Handle<VkPipelineLayout>{ device.handle(), vkDestroyPipelineLayout };

        if (auto result =
              vkCreatePipelineLayout(device.handle(), &pipelineLayoutCreateInfo, nullptr, &outPipelineLayout);
            result != VK_SUCCESS) {
            throw std::runtime_error("could not create graphics pipeline layout");
        }

        auto graphicsPipelineCreateInfo = VkGraphicsPipelineCreateInfo{};
        graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        graphicsPipelineCreateInfo.stageCount = shaderStages.size();
        graphicsPipelineCreateInfo.pStages = shaderStages.data();
        graphicsPipelineCreateInfo.pVertexInputState = &vertexInput;
        graphicsPipelineCreateInfo.pInputAssemblyState = &assemblyState;
        graphicsPipelineCreateInfo.pViewportState = &viewportState;
        graphicsPipelineCreateInfo.pRasterizationState = &rasterizationState;
        graphicsPipelineCreateInfo.pMultisampleState = &multisampleState;
        graphicsPipelineCreateInfo.pColorBlendState = &colorBlendState;
        graphicsPipelineCreateInfo.layout = static_cast<VkPipelineLayout>(outPipelineLayout);
        graphicsPipelineCreateInfo.renderPass = renderPass;
        graphicsPipelineCreateInfo.subpass = subPassIndex;
        graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;

        auto depthStencilStateCreateInfo = VkPipelineDepthStencilStateCreateInfo{};
        if (depthStencilRequired) {
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

        outPipeline = vulkan::Handle<VkPipeline>{ device.handle(), vkDestroyPipeline };

        if (auto result = vkCreateGraphicsPipelines(
              device.handle(), VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, &outPipeline);
            result != VK_SUCCESS) {
            throw std::runtime_error("could not create graphics pipeline");
        }
    }
}
}
