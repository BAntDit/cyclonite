//
// Created by bantdit on 2/15/20.
//

#include "renderPass.h"
#include "vulkan/shaderModule.h"

namespace cyclonite {
RenderPass::FrameCommands::FrameCommands() noexcept
  : transferVersion_{ 0 }
  , graphicsVersion_{ 0 }
  , persistentTransfer_{}
  , transferCommands_{}
  , transferSemaphores_{}
  , transientTransfer_{}
  , transientSemaphores_{}
  , transientDstWaitFlags_{}
  , vkSignalSemaphore_{ VK_NULL_HANDLE }
  , graphicsCommands_{ nullptr }
  , waitSemaphores_{}
  , dstWaitFlags_{}
  , indicesBuffer_{ nullptr }
  , transformBuffer_{ nullptr }
  , commandBuffer_{ nullptr }
  , vkUniformsBuffer_{ VK_NULL_HANDLE }
  , drawCommandCount_{ 0 }
  , descriptorSetLayout_{}
  , pipelineLayout_{}
  , pipeline_{}
  , descriptorSet_{ VK_NULL_HANDLE }
  , transferQueueSubmitInfo_{ nullptr }
  , graphicsQueueSubmitInfo_{}
{}

RenderPass::FrameCommands::FrameCommands(vulkan::Device const& device)
  : transferVersion_{ 0 }
  , graphicsVersion_{ 0 }
  , persistentTransfer_{}
  , transferCommands_{}
  , transferSemaphores_{}
  , transientTransfer_{}
  , transientSemaphores_{}
  , transientDstWaitFlags_{}
  , vkSignalSemaphore_{ VK_NULL_HANDLE }
  , graphicsCommands_{ nullptr }
  , waitSemaphores_{}
  , dstWaitFlags_{}
  , indicesBuffer_{ nullptr }
  , transformBuffer_{ nullptr }
  , commandBuffer_{ nullptr }
  , vkUniformsBuffer_{ VK_NULL_HANDLE }
  , drawCommandCount_{ 0 }
  , descriptorSet_{ VK_NULL_HANDLE }
  , transferQueueSubmitInfo_{ nullptr }
  , graphicsQueueSubmitInfo_{}
{}

void RenderPass::FrameCommands::setFrameSemaphore(VkSemaphore semaphore)
{
    assert(waitSemaphores_.size() > 0);
    waitSemaphores_[0] = semaphore;
}

auto RenderPass::FrameCommands::getWaitSemaphore(size_t semaphoreId, VkPipelineStageFlags flags)
  -> std::pair<size_t, VkSemaphore const*>
{
    if (semaphoreId == std::numeric_limits<size_t>::max()) {
        semaphoreId = semaphores_.size();
    }

    if (semaphores_.count(semaphoreId) == 0) {
        auto [it, _] = semaphores_.emplace(
          semaphoreId, std::make_pair(waitSemaphores_.size(), vulkan::Handle{ vkDevice_, vkDestroySemaphore }));

        auto& [key, value] = *it;
        auto& [idx, handle] = value;

        VkSemaphoreCreateInfo semaphoreCreateInfo = {};
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        if (auto result = vkCreateSemaphore(vkDevice_, &semaphoreCreateInfo, nullptr, &handle); result != VK_SUCCESS) {
            throw std::runtime_error("could not create synchronization semaphore");
        }

        assert(flags != VK_PIPELINE_STAGE_FLAG_BITS_MAX_ENUM);

        waitSemaphores_.emplace_back(static_cast<VkSemaphore>(handle));
        waitFlags_.emplace_back(flags);

        (void)_;
        (void)key;
        (void)idx;
    }

    assert(semaphores_.count(semaphoreId) != 0 && waitSemaphores_.size() < semaphores_[semaphoreId].first);

    return std::make_pair(semaphoreId, waitSemaphores_.data() + semaphores_[semaphoreId].first);
}

void RenderPass::FrameCommands::setUniformBuffer(std::shared_ptr<vulkan::Buffer> const& buffer)
{
    if (uniforms_ != buffer) {
        uniforms_ = buffer;
        _reset();
    }
}

void RenderPass::FrameCommands::_reset()
{
    descriptorSetExpired_ = true;
    graphicsCommands_.reset();
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
                                       VkPipeline pipeline,
                                       VkSemaphore frameBufferAvailableSemaphore,
                                       VkSemaphore passFinishedSemaphore,
                                       bool depthStencilRequired)
{
    if (descriptorSetExpired_ && descriptorSet_ != VK_NULL_HANDLE) {
        vkFreeDescriptorSets(device.handle(), descriptorPool, 1, &descriptorSet_);
        descriptorSet_ = VK_NULL_HANDLE;
    }

    if (descriptorSet_ == VK_NULL_HANDLE) {
        _createDescriptorSets(device.handle(), descriptorPool, descriptorSetLayout);

        // TODO:: write descriptor set

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

              vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

              vkCmdBindDescriptorSets(
                commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet_, 0, nullptr);

              vkCmdDraw(commandBuffer, 3, 1, 0, 0);

              vkCmdEndRenderPass(commandBuffer);

              if (auto result = vkEndCommandBuffer(commandBuffer); result != VK_SUCCESS) {
                  throw std::runtime_error("could not record command buffer!");
              }
          }));
    } // graphics update end

    auto transferVersion = frameUpdate.transferVersion();
    auto graphicsVersion = frameUpdate.graphicsVersion();

    vkSignalSemaphore_ = passFinishedSemaphore;

    if (transferVersion_ != frameUpdate.transferVersion()) {
        _clearTransientTransfer(); // TODO:: move to the end of the frame

        transferQueueSubmitInfo_.reset();

        // persistent
        transferCommands_ = frameUpdate.transferCommands_;
        transferSemaphores_ = frameUpdate.transferSemaphores_;
        waitSemaphores_ = frameUpdate.transferSemaphores_;
        dstWaitFlags_ = frameUpdate.dstWaitFlags_;

        // transient
        if (!frameUpdate.transientTransfer_.empty()) {
            // transient commands stay actual one frame only
            // move transient commands ownership
            transientTransfer_ = std::move(frameUpdate.transientTransfer_);
            transientSemaphores_ = std::move(frameUpdate.transientSemaphores_);
            transientDstWaitFlags_ = std::move(frameUpdate.transientDstWaitFlags_);

            // just because vector has no standard-defined moved-from state
            // so, just in case:
            frameUpdate.transientTransfer_.clear();
            frameUpdate.transientSemaphores_.clear();
            frameUpdate.transientDstWaitFlags_.clear();
            // ...

            for (auto& tt : transientTransfer_) {
                for (size_t i = 0, count = tt->commandBufferCount(); i < count; i++) {
                    transferCommands_.push_back(tt->getCommandBuffer(i));
                }
            }

            transferSemaphores_.reserve(transferSemaphores_.size() + transientSemaphores_.size());
            waitSemaphores_.reserve(waitSemaphores_.size() + transientSemaphores_.size() + 1);
            dstWaitFlags_.reserve(dstWaitFlags_.size() + transientDstWaitFlags_.size() + 1);

            assert(transientSemaphores_.size() == transientDstWaitFlags_.size());

            for (size_t i = 0, count = transientSemaphores_.size(); i < count; i++) {
                transferSemaphores_.push_back(static_cast<VkSemaphore>(transientSemaphores_[i]));
                waitSemaphores_.push_back(static_cast<VkSemaphore>(transientSemaphores_[i]));
                dstWaitFlags_.push_back(transientDstWaitFlags_[i]);
            }

            // up version in advance
            // to reassembly transfer submit with no current frame transient commands next frame
            frameUpdate.transferVersion_++;
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
    } // transfer update end

    if (graphicsVersion_ != frameUpdate.graphicsVersion()) {
        _updatePipeline(device, renderPass, viewport, depthStencilRequired);

        if (commandBuffer_ != frameUpdate.commandBuffer_) {
            commandBuffer_ = frameUpdate.commandBuffer_;
            drawCommandCount_ = frameUpdate.drawCommandCount_;
        }

        if (indicesBuffer_ != frameUpdate.indicesBuffer_) {
            indicesBuffer_ = frameUpdate.indicesBuffer_;
        }

        vkUniformsBuffer_ = frameUpdate.vkUniformsBuffer_;

        if (transformBuffer_ != frameUpdate.transformBuffer_) {
            transformBuffer_ = frameUpdate.transformBuffer_;

            if (descriptorSet_ == VK_NULL_HANDLE) {
                _createDescriptorSets(device.handle(), descriptorPool);
            }

            VkDescriptorBufferInfo transformsBufferInfo = {};

            transformsBufferInfo.buffer = transformBuffer_->handle();
            transformsBufferInfo.offset = 0;
            transformsBufferInfo.range = VK_WHOLE_SIZE;

            VkDescriptorBufferInfo uniformBufferInfo = {};
            uniformBufferInfo.buffer = vkUniformsBuffer_;
            uniformBufferInfo.offset = 0;
            uniformBufferInfo.range = VK_WHOLE_SIZE;

            std::array<VkWriteDescriptorSet, 2> writeDescriptorSets = { VkWriteDescriptorSet{},
                                                                        VkWriteDescriptorSet{} };
            writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeDescriptorSets[0].dstSet = descriptorSet_;
            writeDescriptorSets[0].dstBinding = 0;
            writeDescriptorSets[0].descriptorCount = 1;
            writeDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            writeDescriptorSets[0].pBufferInfo = &transformsBufferInfo;

            writeDescriptorSets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeDescriptorSets[1].dstSet = descriptorSet_;
            writeDescriptorSets[1].dstBinding = 1;
            writeDescriptorSets[1].descriptorCount = 1;
            writeDescriptorSets[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            writeDescriptorSets[1].pBufferInfo = &uniformBufferInfo;

            vkUpdateDescriptorSets(device.handle(), writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
        }

        assert(indicesBuffer_);
        assert(transformBuffer_);
        assert(commandBuffer_);
        assert(vkUniformsBuffer_ != VK_NULL_HANDLE);

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

              vkCmdBindDescriptorSets(commandBuffer,
                                      VK_PIPELINE_BIND_POINT_GRAPHICS,
                                      static_cast<VkPipelineLayout>(pipelineLayout_),
                                      0,
                                      1,
                                      &descriptorSet_,
                                      0,
                                      nullptr);

              vkCmdDrawIndexedIndirect(
                commandBuffer, commandBuffer_->handle(), 0, drawCommandCount_, sizeof(VkDrawIndexedIndirectCommand));

              vkCmdEndRenderPass(commandBuffer);

              if (auto result = vkEndCommandBuffer(commandBuffer); result != VK_SUCCESS) {
                  throw std::runtime_error("could not record command buffer!");
              }
          }));
    } // graphics update end

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

    transferVersion_ = transferVersion;
    graphicsVersion_ = graphicsVersion;
}

void RenderPass::FrameCommands::setIndicesBuffer(std::shared_ptr<vulkan::Buffer> const& buffer)
{
    indicesBuffer_ = buffer;
    graphicsVersion_++;
}

void RenderPass::FrameCommands::setTransferBuffer(std::shared_ptr<vulkan::Buffer> const& buffer)
{
    transformBuffer_ = buffer;
    graphicsVersion_++;
}

void RenderPass::FrameCommands::setCommandBuffer(std::shared_ptr<vulkan::Buffer> const& buffer, uint32_t commandCount)
{
    drawCommandCount_ = commandCount;
    commandBuffer_ = buffer;
    graphicsVersion_++;
}

void RenderPass::FrameCommands::setUniformBuffer(VkBuffer uniforms)
{
    vkUniformsBuffer_ = uniforms;
}
}