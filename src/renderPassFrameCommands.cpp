//
// Created by bantdit on 2/15/20.
//

#include "renderPass.h"
#include "vulkan/shaderModule.h"

std::vector<uint32_t> const defaultVertexShaderCode = {
#include "shaders/default.vert.spv.cpp.txt"
};

std::vector<uint32_t> const defaultFragmentShaderCode = {
#include "shaders/default.frag.spv.cpp.txt"
};

namespace cyclonite {
RenderPass::FrameCommands::FrameCommands() noexcept
  : version_{ 0 }
  , fence_{}
  , passFinishedSemaphore_{}
  , vkSignalSemaphore_{ VK_NULL_HANDLE }
  , drawCommandsTransferCommandsIndex_{ std::numeric_limits<size_t>::max() }
  , graphicsCommands_{ nullptr }
  , waitSemaphores_{}
  , dstWaitFlags_{}
  , transferCommands_{}
  , transferSemaphores_{}
  , transientCommandBuffers_{}
  , transientSemaphores_{}
  , transientDstWaitFlags_{}
  , indicesBuffer_{ nullptr }
  , descriptorSetLayout_{}
  , pipelineLayout_{}
  , pipeline_{}
  , transferQueueSubmitInfo_{ nullptr }
  , graphicsQueueSubmitInfo_{}
{}

RenderPass::FrameCommands::FrameCommands(vulkan::Device const& device)
  : version_{ 0 }
  , fence_{ device.handle(), vkDestroyFence }
  , passFinishedSemaphore_{ device.handle(), vkDestroySemaphore }
  , vkSignalSemaphore_{ VK_NULL_HANDLE }
  , drawCommandsTransferCommandsIndex_{ std::numeric_limits<size_t>::max() }
  , graphicsCommands_{ nullptr }
  , waitSemaphores_{}
  , dstWaitFlags_{}
  , transferCommands_{}
  , transferSemaphores_{}
  , transientCommandBuffers_{}
  , transientSemaphores_{}
  , transientDstWaitFlags_{}
  , indicesBuffer_{ nullptr }
  , descriptorSetLayout_{ device.handle(), vkDestroyDescriptorSetLayout }
  , pipelineLayout_{ device.handle(), vkDestroyPipelineLayout }
  , pipeline_{ device.handle(), vkDestroyPipeline }
  , transferQueueSubmitInfo_{ nullptr }
  , graphicsQueueSubmitInfo_{}
{
    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    if (auto result = vkCreateFence(device.handle(), &fenceCreateInfo, nullptr, &fence_); result != VK_SUCCESS) {
        throw std::runtime_error("could not create frame synchronization fence");
    }

    VkSemaphoreCreateInfo semaphoreCreateInfo = {};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    if (auto result = vkCreateSemaphore(device.handle(), &semaphoreCreateInfo, nullptr, &passFinishedSemaphore_);
        result != VK_SUCCESS) {
        throw std::runtime_error("could not create pass end synchronization semaphore");
    }
}

auto RenderPass::FrameCommands::hasDrawCommandsTransferCommands() const -> bool
{
    return drawCommandsTransferCommandsIndex_ != std::numeric_limits<size_t>::max();
}

void RenderPass::FrameCommands::addTransientTransferCommands(std::unique_ptr<vulkan::BaseCommandBufferSet>&& bufferSet,
                                                             vulkan::Handle<VkSemaphore>&& semaphore,
                                                             VkPipelineStageFlags dstWaitFlag)
{
    transientCommandBuffers_.emplace_back(std::move(bufferSet));
    transientSemaphores_.emplace_back(std::move(semaphore));
    transientDstWaitFlags_.emplace_back(dstWaitFlag);
}

void RenderPass::FrameCommands::_clearTransientTransfer()
{
    // clear out of date transient commands
    transientCommandBuffers_.clear();
    transientSemaphores_.clear();
    transientDstWaitFlags_.clear();
}

void RenderPass::FrameCommands::_updatePipeline(vulkan::Device& device,
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
    vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
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
    rasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizationState.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampleState = {};
    multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleState.sampleShadingEnable = VK_FALSE;
    multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; // TODO:: must come from render target

    VkPipelineColorBlendStateCreateInfo colorBlendState = {}; // TODO:: must come from render target
    colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendState.logicOpEnable = VK_FALSE;
    colorBlendState.logicOp = VK_LOGIC_OP_COPY;

    std::array<VkDescriptorSetLayoutBinding, 1> bindings = { VkDescriptorSetLayoutBinding{} };
    bindings[0].binding = 0;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    bindings[0].descriptorCount = 0;
    bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

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
    pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout_;

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
        depthStencilStateCreateInfo.depthTestEnable = VK_FALSE;
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

void RenderPass::FrameCommands::update(vulkan::Device& device,
                                       VkRenderPass renderPass,
                                       VkFramebuffer framebuffer,
                                       std::array<uint32_t, 4>&& viewport,
                                       VkSemaphore frameBufferAvailableSemaphore,
                                       VkSemaphore passFinishedSemaphore,
                                       std::pair<size_t, VkClearValue const*>&& clearValues,
                                       bool depthStencilRequired,
                                       FrameCommands& frameUpdate)
{
    auto version = frameUpdate.version();

    vkSignalSemaphore_ = passFinishedSemaphore;

    if (indicesBuffer_ != frameUpdate.indicesBuffer_) {
        indicesBuffer_ = frameUpdate.indicesBuffer_;
    }

    assert(indicesBuffer_);

    if (transferSubmitVersion() != frameUpdate.transferSubmitVersion()) {
        _clearTransientTransfer();

        transferQueueSubmitInfo_.reset();

        transferCommands_ = frameUpdate.transferCommands_;
        transferSemaphores_ = frameUpdate.transferSemaphores_;
        waitSemaphores_ = frameUpdate.transferSemaphores_;
        dstWaitFlags_ = frameUpdate.dstWaitFlags_;

        if (!frameUpdate.transientCommandBuffers_.empty()) {
            // transient commands stay actual one frame only
            // move transient commands ownership
            transientCommandBuffers_ = std::move(frameUpdate.transientCommandBuffers_);
            transientSemaphores_ = std::move(frameUpdate.transientSemaphores_);
            transientDstWaitFlags_ = std::move(frameUpdate.transientDstWaitFlags_);

            // just because vector has no standard-defined moved-from state
            // so, just in case:
            frameUpdate.transientCommandBuffers_.clear();
            frameUpdate.transientSemaphores_.clear();
            frameUpdate.transientDstWaitFlags_.clear();
            // ...

            transferSemaphores_.reserve(transferSemaphores_.size() + transientSemaphores_.size());
            waitSemaphores_.reserve(waitSemaphores_.size() + transientSemaphores_.size() + 1);
            dstWaitFlags_.reserve(dstWaitFlags_.size() + transientDstWaitFlags_.size() + 1);

            for (size_t i = 0, count = transientCommandBuffers_.size(); i < count; i++) {
                transferSemaphores_.push_back(static_cast<VkSemaphore>(transientSemaphores_[i]));
                waitSemaphores_.push_back(static_cast<VkSemaphore>(transientSemaphores_[i]));
                dstWaitFlags_.push_back(transientDstWaitFlags_[i]);

                for (size_t k = 0, bufferCount = transientCommandBuffers_[i]->commandBufferCount(); k < bufferCount;
                     k++) {
                    transferCommands_.push_back(transientCommandBuffers_[i]->getCommandBuffer(k));
                }
            }

            // up version in advance
            // to reassembly transfer submit with no current frame transient commands next frame
            frameUpdate.version_ = uint64_t{ static_cast<uint64_t>(frameUpdate.transferSubmitVersion() + 1) |
                                             static_cast<uint64_t>(frameUpdate.graphicsSubmitVersion()) << 32UL };
        }

        if (!transferCommands_.empty()) {
            transferQueueSubmitInfo_ = std::make_unique<VkSubmitInfo>(VkSubmitInfo{});

            transferQueueSubmitInfo_->sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            transferQueueSubmitInfo_->commandBufferCount = static_cast<uint32_t>(transferCommands_.size());
            transferQueueSubmitInfo_->pCommandBuffers = transferCommands_.data();
            transferQueueSubmitInfo_->signalSemaphoreCount = static_cast<uint32_t>(transferSemaphores_.size());
            transferQueueSubmitInfo_->pSignalSemaphores = transferSemaphores_.data();
        }

        dstWaitFlags_.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
        waitSemaphores_.push_back(VK_NULL_HANDLE);
    }

    if (graphicsSubmitVersion() != frameUpdate.graphicsSubmitVersion()) {
        _updatePipeline(device, renderPass, viewport, depthStencilRequired);

        graphicsCommands_ = std::make_unique<graphics_queue_commands_t>(device.commandPool().allocCommandBuffers(
          vulkan::CommandBufferSet<vulkan::CommandPool, std::array<VkCommandBuffer, 1>>{
            device.graphicsQueueFamilyIndex(),
            VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            std::array<VkCommandBuffer, 1>{} },
          [=](auto&& graphicsCommands) -> void {
              auto [commandBuffer] = graphicsCommands;
              auto [x, y, width, height] = viewport;
              auto [clearValueCount, pClearValues] = clearValues;

              VkCommandBufferBeginInfo commandBufferBeginInfo = {};
              commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

              if (vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo) != VK_SUCCESS) {
                  throw std::runtime_error("could not begin recording command buffer!");
              }

              VkRenderPassBeginInfo renderPassBeginInfo = {};
              renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
              renderPassBeginInfo.renderPass = renderPass;
              renderPassBeginInfo.framebuffer = framebuffer;
              renderPassBeginInfo.renderArea.offset.x = x;
              renderPassBeginInfo.renderArea.offset.y = y;
              renderPassBeginInfo.renderArea.extent.width = width;
              renderPassBeginInfo.renderArea.extent.height = height;
              renderPassBeginInfo.clearValueCount = clearValueCount;
              renderPassBeginInfo.pClearValues = pClearValues;

              vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

              vkCmdBindIndexBuffer(commandBuffer, indicesBuffer_->handle(), 0, VK_INDEX_TYPE_UINT32);

              vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, static_cast<VkPipeline>(pipeline_));

              // TODO:: THE MAIN TODO for the next time!!!

              vkCmdEndRenderPass(commandBuffer);

              if (auto result = vkEndCommandBuffer(commandBuffer); result != VK_SUCCESS) {
                  throw std::runtime_error("could not record command buffer!");
              }
          }));
    }

    assert(!waitSemaphores_.empty());

    waitSemaphores_.back() = frameBufferAvailableSemaphore;

    std::fill_n(&graphicsQueueSubmitInfo_, 1, VkSubmitInfo{});

    graphicsQueueSubmitInfo_.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    graphicsQueueSubmitInfo_.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores_.size());
    graphicsQueueSubmitInfo_.pWaitSemaphores = waitSemaphores_.data();
    graphicsQueueSubmitInfo_.pWaitDstStageMask = dstWaitFlags_.data();
    graphicsQueueSubmitInfo_.commandBufferCount = static_cast<uint32_t>(graphicsCommands_->commandBufferCount());
    graphicsQueueSubmitInfo_.pCommandBuffers = graphicsCommands_->pCommandBuffers();
    graphicsQueueSubmitInfo_.signalSemaphoreCount = 1;
    graphicsQueueSubmitInfo_.pSignalSemaphores = &vkSignalSemaphore_;

    version_ = version;
}
}