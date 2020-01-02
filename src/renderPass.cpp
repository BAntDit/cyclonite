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
// template<size_t presentModeCandidateCount, typename... DepthStencilOutputCandidates, typename...
// ColorOutputCandidates>
RenderPass::RenderPass(vulkan::Device& device,
                       Options::WindowProperties const& windowProperties,
                       VkCompositeAlphaFlagBitsKHR vkCompositeAlphaFlags
                       // std::array<VkPresentModeKHR, presentModeCandidateCount> const& presentModeCandidates,
                       // render_target_output<type_list<DepthStencilOutputCandidates...>> const&,
                       // render_target_output<type_list<ColorOutputCandidates...>> const&
                       )
  : vkRenderPass_{ device.handle(), vkDestroyRenderPass }
  , renderTarget_{ nullptr }
  , passFinishedSemaphores_{}
  , frameFences_{}
  , renderTargetFences_{}
  , renderQueueSubmitInfo_{}
  , vkDummyPipelineLayout_{ device.handle(), vkDestroyPipelineLayout }
  , vkDummyPipeline_{ device.handle(), vkDestroyPipeline }
  , vkCommandPool_{ device.handle(), vkDestroyCommandPool }
  , commandBuffers_{}
{
    std::array<VkPresentModeKHR, 2> presentModeCandidates = { VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_FIFO_KHR };

    using rt_builder_t = RenderTargetBuilder<
      render_target_output<type_list<render_target_output_candidate<VK_FORMAT_D32_SFLOAT>>>,
      render_target_output<
        type_list<render_target_output_candidate<VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR>>,
        RenderTargetOutputSemantic::DEFAULT>>;

    rt_builder_t rtBuilder{ device, Surface{ device, windowProperties }, presentModeCandidates, vkCompositeAlphaFlags };

    auto [attachments, references] = rtBuilder.getAttachments();

    VkSubpassDescription subPass = {};
    subPass.colorAttachmentCount = rt_builder_t::color_attachment_count_v;
    subPass.pColorAttachments = references.data();

    // if constexpr (sizeof...(DepthStencilOutputCandidates) > 0) {
    subPass.pDepthStencilAttachment = &references[rt_builder_t::depth_attachment_idx_v];
    // }

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = attachments.size();
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subPass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (auto result = vkCreateRenderPass(device.handle(), &renderPassInfo, nullptr, &vkRenderPass_);
        result != VK_SUCCESS) {

        if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
            throw std::runtime_error("not enough RAM memory to create render pass");
        }

        if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
            throw std::runtime_error("not enough GPU memory to create render pass");
        }

        assert(false);
    }

    renderTarget_ =
      std::make_unique<RenderTarget>(rtBuilder.buildRenderPassTarget(static_cast<VkRenderPass>(vkRenderPass_)));

    frameFences_.reserve(renderTarget_->swapChainLength());
    passFinishedSemaphores_.reserve(renderTarget_->swapChainLength());

    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkSemaphoreCreateInfo semaphoreCreateInfo = {};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    for (size_t i = 0, count = renderTarget_->swapChainLength(); i < count; i++) {
        if (auto result = vkCreateFence(
              device.handle(), &fenceCreateInfo, nullptr, &frameFences_.emplace_back(device.handle(), vkDestroyFence));
            result != VK_SUCCESS) {
            throw std::runtime_error("could not create frame synchronization fence");
        }

        if (auto result = vkCreateSemaphore(device.handle(),
                                            &semaphoreCreateInfo,
                                            nullptr,
                                            &passFinishedSemaphores_.emplace_back(device.handle(), vkDestroySemaphore));
            result != VK_SUCCESS) {
            throw std::runtime_error("could not create pass end synchronization semaphore");
        }
    }

    renderTargetFences_.resize(renderTarget_->swapChainLength(), VK_NULL_HANDLE);

    _createDummyPipeline(device);

    _createDummyCommandPool(device);

    commandBuffers_.resize(renderTarget_->swapChainLength(), VK_NULL_HANDLE);

    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.commandPool = static_cast<VkCommandPool>(vkCommandPool_);
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers_.size());

    if (auto result = vkAllocateCommandBuffers(device.handle(), &commandBufferAllocateInfo, commandBuffers_.data());
        result != VK_SUCCESS) {
        std::runtime_error("could not allocate command buffers");
    }

    VkClearValue clearColor = { { { 0.0f, 0.0f, 0.0f, 1.0f } } };

    for (size_t i = 0, count = commandBuffers_.size(); i < count; i++) {
        auto commandBuffer = commandBuffers_[i];

        VkCommandBufferBeginInfo commandBufferBeginInfo = {};
        commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo) != VK_SUCCESS) {
            throw std::runtime_error("could not begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassBeginInfo = {};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = static_cast<VkRenderPass>(vkRenderPass_);
        renderPassBeginInfo.framebuffer = renderTarget_->frameBuffers()[i].handle();
        renderPassBeginInfo.renderArea.offset.x = 0;
        renderPassBeginInfo.renderArea.offset.y = 0;
        renderPassBeginInfo.renderArea.extent.width = renderTarget_->width();
        renderPassBeginInfo.renderArea.extent.height = renderTarget_->height();
        renderPassBeginInfo.clearValueCount = 1;
        renderPassBeginInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, static_cast<VkPipeline>(vkDummyPipeline_));

        vkCmdDraw(commandBuffer, 3, 1, 0, 0);

        vkCmdEndRenderPass(commandBuffer);

        if (auto result = vkEndCommandBuffer(commandBuffer); result != VK_SUCCESS) {
            throw std::runtime_error("could not record command buffer!");
        }
    }
}

auto RenderPass::begin(vulkan::Device const& device) -> VkFence
{
    auto frontBufferIndex = renderTarget_->frontBufferIndex();

    vkWaitForFences(device.handle(), 1, &frameFences_[frontBufferIndex], VK_TRUE, std::numeric_limits<uint64_t>::max());

    auto backBufferIndex = renderTarget_->acquireBackBufferIndex(device);

    if (renderTargetFences_[backBufferIndex] != VK_NULL_HANDLE) {
        vkWaitForFences(
          device.handle(), 1, &renderTargetFences_[backBufferIndex], VK_TRUE, std::numeric_limits<uint64_t>::max());
    }

    vkResetFences(device.handle(), 1, &frameFences_[frontBufferIndex]);

    return renderTargetFences_[backBufferIndex] = static_cast<VkFence>(frameFences_[frontBufferIndex]);
}

void RenderPass::end(vulkan::Device const& device)
{
    auto frontBufferIndex = renderTarget_->frontBufferIndex();

    renderTarget_->swapBuffers(device, static_cast<VkSemaphore>(passFinishedSemaphores_[frontBufferIndex]));
}

void RenderPass::_createDummyPipeline(vulkan::Device const& device)
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

void RenderPass::_createDummyCommandPool(vulkan::Device const& device)
{
    VkCommandPoolCreateInfo commandPoolCreateInfo = {};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.queueFamilyIndex = device.graphicsQueueFamilyIndex();

    if (auto result = vkCreateCommandPool(device.handle(), &commandPoolCreateInfo, nullptr, &vkCommandPool_);
        result != VK_SUCCESS) {
        throw std::runtime_error("could not create command pool");
    }
}

auto RenderPass::renderQueueSubmitInfo() -> VkSubmitInfo const&
{
    auto backBufferIndex = renderTarget_->backBufferIndex();
    auto frontBufferIndex = renderTarget_->frontBufferIndex();

    memset(&renderQueueSubmitInfo_, 0, sizeof(renderQueueSubmitInfo_));

    std::array<VkPipelineStageFlags, 1> waitStages = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    std::array<VkSemaphore, 1> waitSemaphores = { renderTarget_->frontBufferAvailableSemaphore() };
    std::array<VkSemaphore, 1> signalSemaphores = { static_cast<VkSemaphore>(
      passFinishedSemaphores_[frontBufferIndex]) };

    renderQueueSubmitInfo_.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    renderQueueSubmitInfo_.waitSemaphoreCount = waitSemaphores.size();
    renderQueueSubmitInfo_.pWaitSemaphores = waitSemaphores.data();
    renderQueueSubmitInfo_.pWaitDstStageMask = waitStages.data();

    renderQueueSubmitInfo_.commandBufferCount = 1;
    renderQueueSubmitInfo_.pCommandBuffers = &commandBuffers_[backBufferIndex];

    renderQueueSubmitInfo_.signalSemaphoreCount = signalSemaphores.size();
    renderQueueSubmitInfo_.pSignalSemaphores = signalSemaphores.data();

    return renderQueueSubmitInfo_;
}
}
