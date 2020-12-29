//
// Created by bantdit on 2/15/20.
//

#include "renderPass.h"
#include <iostream>

namespace cyclonite::render {
RenderPass::FrameCommands::FrameCommands(vulkan::Device const& device)
  : vkDevice_{ device.handle() }
  , indices_{ nullptr }
  , vertices_{ nullptr }
  , instances_{ nullptr }
  , commands_{ nullptr }
  , uniforms_{ nullptr }
  , commandCount_{ 0 }
  , waitSemaphores_(1, VK_NULL_HANDLE)
  , waitFlags_(1, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT)
  , waitSemaphoreCount_{ 1 }
  , graphicsCommands_{ nullptr }
  , descriptorSetExpired_{ false }
  , descriptorSet_{ VK_NULL_HANDLE }
{}

void RenderPass::FrameCommands::resetWaitSemaphores()
{
    waitSemaphoreCount_ = 1;
}

void RenderPass::FrameCommands::addWaitSemaphore(VkSemaphore waitSemaphore,
                                                 VkPipelineStageFlags flags /* = VK_PIPELINE_STAGE_FLAG_BITS_MAX_ENUM*/)
{
    auto idx = waitSemaphoreCount_++;

    if (waitSemaphores_.size() < waitSemaphoreCount_) {
        waitSemaphores_.resize(waitSemaphoreCount_, VK_NULL_HANDLE);
        waitFlags_.resize(waitSemaphoreCount_, VK_PIPELINE_STAGE_FLAG_BITS_MAX_ENUM);
    }

    waitSemaphores_[idx] = waitSemaphore;
    waitFlags_[idx] = flags;
}

void RenderPass::FrameCommands::setUniformBuffer(std::shared_ptr<vulkan::Buffer> const& buffer)
{
    if (uniforms_ != buffer) {
        uniforms_ = buffer;
        descriptorSetExpired_ = true;
        graphicsCommands_.reset();
    }
}

void RenderPass::FrameCommands::setIndexBuffer(std::shared_ptr<vulkan::Buffer> const& buffer)
{
    if (indices_ != buffer) {
        indices_ = buffer;
        graphicsCommands_.reset();
    }
}

void RenderPass::FrameCommands::setVertexBuffer(std::shared_ptr<vulkan::Buffer> const& buffer)
{
    if (vertices_ != buffer) {
        vertices_ = buffer;
        descriptorSetExpired_ = true;
        graphicsCommands_.reset();
    }
}

void RenderPass::FrameCommands::setInstanceBuffer(std::shared_ptr<vulkan::Buffer> const& buffer)
{
    if (instances_ != buffer) {
        instances_ = buffer;
        descriptorSetExpired_ = true;
        graphicsCommands_.reset();
    }
}

void RenderPass::FrameCommands::setCommandBuffer(std::shared_ptr<vulkan::Buffer> const& buffer, uint32_t commandCount)
{
    bool resetCommands = false;

    if (commands_ != buffer) {
        commands_ = buffer;
        resetCommands = true;
    }

    if (commandCount_ != commandCount) {
        commandCount_ = commandCount;
        resetCommands = true;
    }

    if (resetCommands) {
        graphicsCommands_.reset();
    }
}

void RenderPass::FrameCommands::_createDescriptorSets(VkDevice vkDevice,
                                                      VkDescriptorPool vkDescriptorPool,
                                                      VkDescriptorSetLayout descriptorSetLayout)
{
    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
    descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocateInfo.descriptorPool = vkDescriptorPool;
    descriptorSetAllocateInfo.descriptorSetCount = 1;
    descriptorSetAllocateInfo.pSetLayouts = &descriptorSetLayout; // it's ok, allocation goes in the local scope

    if (auto result = vkAllocateDescriptorSets(vkDevice, &descriptorSetAllocateInfo, &descriptorSet_);
        result != VK_SUCCESS) {
        if (result == VK_ERROR_OUT_OF_HOST_MEMORY)
            throw std::runtime_error("can not allocate descriptor set, out of host memory");

        if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY)
            throw std::runtime_error("can not allocate descriptor set , out of device memory");

        if (result == VK_ERROR_FRAGMENTED_POOL)
            throw std::runtime_error("can not allocate descriptor set, fragment pool");

        if (result == VK_ERROR_OUT_OF_POOL_MEMORY)
            throw std::runtime_error("can not allocate descriptor set, out of pool memory");

        assert(false);
    }
}

void RenderPass::FrameCommands::update(vulkan::Device& device,
                                       VkRenderPass renderPass,
                                       VkFramebuffer framebuffer,
                                       std::array<uint32_t, 4> const& viewport,
                                       std::pair<size_t, VkClearValue const*> const& clearValues,
                                       VkDescriptorPool descriptorPool,
                                       VkDescriptorSetLayout descriptorSetLayout,
                                       VkPipelineLayout pipelineLayout,
                                       VkPipeline pipeline)
{
    if (descriptorSetExpired_ && descriptorSet_ != VK_NULL_HANDLE) {
        vkFreeDescriptorSets(device.handle(), descriptorPool, 1, &descriptorSet_);
        descriptorSet_ = VK_NULL_HANDLE;
    }

    if (descriptorSet_ == VK_NULL_HANDLE) {
        _createDescriptorSets(device.handle(), descriptorPool, descriptorSetLayout);

        VkDescriptorBufferInfo vertexBufferInfo = {};
        vertexBufferInfo.buffer = vertices_->handle();
        vertexBufferInfo.offset = 0;
        vertexBufferInfo.range = VK_WHOLE_SIZE;

        VkDescriptorBufferInfo instanceBufferInfo = {};
        instanceBufferInfo.buffer = instances_->handle();
        instanceBufferInfo.offset = 0;
        instanceBufferInfo.range = VK_WHOLE_SIZE;

        VkDescriptorBufferInfo uniformBufferInfo = {};
        uniformBufferInfo.buffer = uniforms_->handle();
        uniformBufferInfo.offset = 0;
        uniformBufferInfo.range = VK_WHOLE_SIZE;

        std::array<VkWriteDescriptorSet, 3> writeDescriptorSets = { VkWriteDescriptorSet{},
                                                                    VkWriteDescriptorSet{},
                                                                    VkWriteDescriptorSet{} };

        writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSets[0].dstSet = descriptorSet_;
        writeDescriptorSets[0].dstBinding = 0;
        writeDescriptorSets[0].descriptorCount = 1;
        writeDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        writeDescriptorSets[0].pBufferInfo = &vertexBufferInfo;
        writeDescriptorSets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSets[1].dstSet = descriptorSet_;
        writeDescriptorSets[1].dstBinding = 1;
        writeDescriptorSets[1].descriptorCount = 1;
        writeDescriptorSets[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        writeDescriptorSets[1].pBufferInfo = &instanceBufferInfo;
        writeDescriptorSets[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSets[2].dstSet = descriptorSet_;
        writeDescriptorSets[2].dstBinding = 2;
        writeDescriptorSets[2].descriptorCount = 1;
        writeDescriptorSets[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writeDescriptorSets[2].pBufferInfo = &uniformBufferInfo;

        vkUpdateDescriptorSets(device.handle(), writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);

        descriptorSetExpired_ = false;
    }

    if (!graphicsCommands_) {
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

              vkCmdBindIndexBuffer(commandBuffer, indices_->handle(), 0, VK_INDEX_TYPE_UINT32);

              vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

              vkCmdBindDescriptorSets(
                commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet_, 0, nullptr);

              vkCmdDrawIndexedIndirect(
                commandBuffer, commands_->handle(), 0, commandCount_, sizeof(VkDrawIndexedIndirectCommand));

              vkCmdEndRenderPass(commandBuffer);

              if (auto result = vkEndCommandBuffer(commandBuffer); result != VK_SUCCESS) {
                  throw std::runtime_error("could not record command buffer!");
              }
          }));
    } // graphics update end
}
}