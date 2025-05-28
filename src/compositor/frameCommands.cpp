//
// Created by anton on 11/11/21.
//

#include "frameCommands.h"
#include "baseRenderTarget.h"
#include "links.h"
#include "passIterator.h"

namespace cyclonite::compositor {
static void _createDescriptorSet(VkDevice vkDevice,
                                 VkDescriptorPool vkDescriptorPool,
                                 VkDescriptorSetLayout descriptorSetLayout,
                                 VkDescriptorSet* descriptorSet)
{
    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
    descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocateInfo.descriptorPool = vkDescriptorPool;
    descriptorSetAllocateInfo.descriptorSetCount = 1;
    descriptorSetAllocateInfo.pSetLayouts = &descriptorSetLayout; // it's ok, allocation goes in the local scope

    if (auto result = vkAllocateDescriptorSets(vkDevice, &descriptorSetAllocateInfo, descriptorSet);
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

FrameCommands::FrameCommands() noexcept
  : bufferIndex_{ 0 }
  , indices_{}
  , vertices_{}
  , instances_{}
  , commands_{}
  , uniforms_{}
  , commandCount_{}
  , graphicsCommands_{}
{
}

FrameCommands::FrameCommands(size_t bufferIndex) noexcept
  : FrameCommands()
{
    bufferIndex_ = bufferIndex;
}

void FrameCommands::setIndexBuffer(VkQueue graphicQueue, std::shared_ptr<vulkan::Buffer> const& indices)
{
    if (indices_ != indices) {
        _resetCommands(graphicQueue);
        indices_ = indices;
    }
}

void FrameCommands::setVertexBuffer(VkQueue graphicQueue, std::shared_ptr<vulkan::Buffer> const& vertices)
{
    if (vertices_ != vertices) {
        _resetCommands(graphicQueue);
        vertices_ = vertices;
    }
}

void FrameCommands::setInstanceBuffer(VkQueue graphicQueue, std::shared_ptr<vulkan::Buffer> const& instances)
{
    if (instances_ != instances) {
        _resetCommands(graphicQueue);
        instances_ = instances;
    }
}

void FrameCommands::setCommandBuffer(VkQueue graphicQueue,
                                     std::shared_ptr<vulkan::Buffer> const& commands,
                                     uint32_t commandCount)
{
    if (commands != commands_) {
        _resetCommands(graphicQueue);
        commands_ = commands;
        commandCount_ = commandCount;
    }
}

void FrameCommands::setUniformBuffer(VkQueue graphicQueue, std::shared_ptr<vulkan::Buffer> const& uniforms)
{
    if (uniforms_ != uniforms) {
        _resetCommands(graphicQueue);
        uniforms_ = uniforms;
    }
}

void FrameCommands::update(vulkan::Device& device,
                           BaseRenderTarget const& renderTarget,
                           VkRenderPass vkRenderPass,
                           Links const& links,
                           PassIterator const& begin,
                           PassIterator const& end)
{
    assert(multithreading::Render::isInRenderThread());

    for (auto it = begin; it != end; it++) {
        auto [passType, descriptorPool, descriptorSetLayout, pipelineLayout, pipeline, descriptorSetPtr, isExpired] =
          *it;
        (void)pipelineLayout;
        (void)pipeline;

        auto isNew = false;
        if (*descriptorSetPtr == VK_NULL_HANDLE) {
            _createDescriptorSet(device.handle(), descriptorPool, descriptorSetLayout, descriptorSetPtr);
            isNew = true;
        }

        if (isNew || isExpired) {
            constexpr auto maxBufferDescriptorCount = size_t{ 3 };
            constexpr auto maxImageDescriptorCount = size_t{ 32 };
            constexpr auto maxDescriptorCount = maxBufferDescriptorCount + maxImageDescriptorCount;

            auto bufferDescriptors = std::array<VkDescriptorBufferInfo, maxBufferDescriptorCount>{};
            auto imageDescriptors = std::array<VkDescriptorImageInfo, maxImageDescriptorCount>{};
            auto writeDescriptorSets = std::array<VkWriteDescriptorSet, maxDescriptorCount>{};

            auto bufferDescriptorCount = uint32_t{ 0 };
            auto imageDescriptorCount = uint32_t{ 0 };

            if (passType == compositor::PassType::SCENE) {
                if (vertices_) {
                    bufferDescriptors[bufferDescriptorCount].buffer = vertices_->handle();
                    bufferDescriptors[bufferDescriptorCount].offset = 0;
                    bufferDescriptors[bufferDescriptorCount].range = VK_WHOLE_SIZE;

                    auto setIdx = bufferDescriptorCount;
                    writeDescriptorSets[setIdx].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    writeDescriptorSets[setIdx].dstSet = *descriptorSetPtr;
                    writeDescriptorSets[setIdx].dstBinding = bufferDescriptorCount;
                    writeDescriptorSets[setIdx].descriptorCount = 1;
                    writeDescriptorSets[setIdx].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    writeDescriptorSets[setIdx].pBufferInfo = bufferDescriptors.data() + bufferDescriptorCount++;
                }

                if (instances_) {
                    bufferDescriptors[bufferDescriptorCount].buffer = instances_->handle();
                    bufferDescriptors[bufferDescriptorCount].offset = 0;
                    bufferDescriptors[bufferDescriptorCount].range = VK_WHOLE_SIZE;

                    auto setIdx = bufferDescriptorCount;
                    writeDescriptorSets[setIdx].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    writeDescriptorSets[setIdx].dstSet = *descriptorSetPtr;
                    writeDescriptorSets[setIdx].dstBinding = bufferDescriptorCount;
                    writeDescriptorSets[setIdx].descriptorCount = 1;
                    writeDescriptorSets[setIdx].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    writeDescriptorSets[setIdx].pBufferInfo = bufferDescriptors.data() + bufferDescriptorCount++;
                }

                if (uniforms_) {
                    bufferDescriptors[bufferDescriptorCount].buffer = uniforms_->handle();
                    bufferDescriptors[bufferDescriptorCount].offset = 0;
                    bufferDescriptors[bufferDescriptorCount].range = VK_WHOLE_SIZE;

                    auto setIdx = bufferDescriptorCount;
                    writeDescriptorSets[setIdx].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    writeDescriptorSets[setIdx].dstSet = *descriptorSetPtr;
                    writeDescriptorSets[setIdx].dstBinding = bufferDescriptorCount;
                    writeDescriptorSets[setIdx].descriptorCount = 1;
                    writeDescriptorSets[setIdx].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    writeDescriptorSets[setIdx].pBufferInfo = bufferDescriptors.data() + bufferDescriptorCount++;
                }
            } // scene type

            auto descriptorSetIdx = bufferDescriptorCount;
            for (auto i = size_t{ 0 }, count = links.size(); i < count; i++) {
                auto&& [idx, sampler, views, semantics] = links.get(i);

                for (auto j = size_t{ 0 }; j < metrix::value_cast(RenderTargetOutputSemantic::COUNT); j++) {
                    auto semantic = semantics[j];

                    if (semantic == RenderTargetOutputSemantic::INVALID)
                        continue;

                    imageDescriptors[imageDescriptorCount].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    imageDescriptors[imageDescriptorCount].imageView = views[j];
                    imageDescriptors[imageDescriptorCount].sampler = sampler;

                    auto& writeDescriptorSet = writeDescriptorSets[descriptorSetIdx];

                    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    writeDescriptorSet.dstSet = *descriptorSetPtr;
                    writeDescriptorSet.dstBinding = descriptorSetIdx++;
                    writeDescriptorSet.descriptorCount = 1;
                    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    writeDescriptorSet.pImageInfo = imageDescriptors.data() + imageDescriptorCount++;
                }
            } // links

            vkUpdateDescriptorSets(
              device.handle(), bufferDescriptorCount + imageDescriptorCount, writeDescriptorSets.data(), 0, nullptr);
        }
    }

    if (!graphicsCommands_) {
        graphicsCommands_ = std::make_unique<graphics_queue_commands_t>(device.commandPool().allocCommandBuffers(
          vulkan::CommandBufferSet<vulkan::CommandPool, std::array<VkCommandBuffer, 1>>{
            device.graphicsQueueFamilyIndex(),
            VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            std::array<VkCommandBuffer, 1>{} },
          [&](auto&& graphicsCommands) -> void {
              auto [commandBuffer] = graphicsCommands;

              VkCommandBufferBeginInfo commandBufferBeginInfo = {};
              commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
              // TODO:: fix - make this flag unnecessary
              // better fix sync
              commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

              if (vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo) != VK_SUCCESS) {
                  throw std::runtime_error("could not begin recording command buffer!");
              }

              VkRenderPassBeginInfo renderPassBeginInfo = {};
              renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
              renderPassBeginInfo.renderPass = vkRenderPass;
              renderPassBeginInfo.framebuffer = renderTarget.frameBuffer(bufferIndex_).handle();
              renderPassBeginInfo.renderArea.offset.x = 0;
              renderPassBeginInfo.renderArea.offset.y = 0;
              renderPassBeginInfo.renderArea.extent.width = renderTarget.width();
              renderPassBeginInfo.renderArea.extent.height = renderTarget.height();
              renderPassBeginInfo.clearValueCount = renderTarget.clearValues().size();
              renderPassBeginInfo.pClearValues = renderTarget.clearValues().data();

              vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

              // all passes:
              auto passIndex = size_t{ 0 };
              for (auto it = begin; it != end; it++) { // subpasses
                  auto [passType,
                        descriptorPool,
                        descriptorSetLayout,
                        pipelineLayout,
                        pipeline,
                        descriptorSetPtr,
                        isExpired] = *it;

                  (void)descriptorPool;
                  (void)descriptorSetLayout;

                  if (passIndex != 0) {
                      vkCmdNextSubpass(commandBuffer, VK_SUBPASS_CONTENTS_INLINE);
                  }

                  if (passType == compositor::PassType::SCENE) {
                      assert(indices_);
                      assert(commands_);

                      vkCmdBindIndexBuffer(commandBuffer, indices_->handle(), 0, VK_INDEX_TYPE_UINT32);
                      vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
                      vkCmdBindDescriptorSets(commandBuffer,
                                              VK_PIPELINE_BIND_POINT_GRAPHICS,
                                              pipelineLayout,
                                              0,
                                              1,
                                              descriptorSetPtr,
                                              0,
                                              nullptr);
                      vkCmdDrawIndexedIndirect(
                        commandBuffer, commands_->handle(), 0, commandCount_, sizeof(VkDrawIndexedIndirectCommand));
                  } else if (passType == compositor::PassType::SCREEN) {
                      vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
                      vkCmdBindDescriptorSets(commandBuffer,
                                              VK_PIPELINE_BIND_POINT_GRAPHICS,
                                              pipelineLayout,
                                              0,
                                              1,
                                              descriptorSetPtr,
                                              0,
                                              nullptr);
                      vkCmdDraw(commandBuffer, 3, 1, 0, 0);
                  } else {
                      // not implemented pass types
                  }

                  passIndex++;
              } // pass (subpass) cycle

              vkCmdEndRenderPass(commandBuffer);

              if (auto result = vkEndCommandBuffer(commandBuffer); result != VK_SUCCESS) {
                  throw std::runtime_error("could not record command buffer!");
              }
          }));
    } // graphics commands
}

void FrameCommands::_resetCommands(VkQueue graphicQueue)
{
    if (graphicsCommands_) {
        if (auto result = vkQueueWaitIdle(graphicQueue); result != VK_SUCCESS) {
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

        graphicsCommands_.reset(nullptr);
    }
}
}
