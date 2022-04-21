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
  : swapChainIndex_{ 0 }
  , indices_{}
  , vertices_{}
  , instances_{}
  , commands_{}
  , uniforms_{}
  , commandCount_{}
  , graphicsCommands_{}
{}

FrameCommands::FrameCommands(size_t swapChainIndex) noexcept
  : FrameCommands()
{
    swapChainIndex_ = swapChainIndex;
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

              if (vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo) != VK_SUCCESS) {
                  throw std::runtime_error("could not begin recording command buffer!");
              }

              VkRenderPassBeginInfo renderPassBeginInfo = {};
              renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
              renderPassBeginInfo.renderPass = vkRenderPass;
              renderPassBeginInfo.framebuffer = renderTarget.frameBuffer(swapChainIndex_).handle();
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
                        flagsPtr] = *it;

                  auto& flags = *(flagsPtr + (swapChainIndex_ / CHAR_BIT));
                  auto mask = static_cast<std::byte>(1 << (swapChainIndex_ % CHAR_BIT));
                  bool isExpired = (flags & mask) != std::byte{ 0 };

                  if (isExpired && *descriptorSetPtr != VK_NULL_HANDLE) {
                      vkFreeDescriptorSets(device.handle(), descriptorPool, 1, descriptorSetPtr);
                      *descriptorSetPtr = VK_NULL_HANDLE;
                  }

                  if (*descriptorSetPtr == VK_NULL_HANDLE) {
                      _createDescriptorSet(device.handle(), descriptorPool, descriptorSetLayout, descriptorSetPtr);

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

                              writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                              writeDescriptorSets[0].dstSet = *descriptorSetPtr;
                              writeDescriptorSets[0].dstBinding = bufferDescriptorCount;
                              writeDescriptorSets[0].descriptorCount = 1;
                              writeDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                              writeDescriptorSets[0].pBufferInfo = bufferDescriptors.data() + bufferDescriptorCount++;
                          }

                          if (instances_) {
                              bufferDescriptors[bufferDescriptorCount].buffer = instances_->handle();
                              bufferDescriptors[bufferDescriptorCount].offset = 0;
                              bufferDescriptors[bufferDescriptorCount].range = VK_WHOLE_SIZE;

                              writeDescriptorSets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                              writeDescriptorSets[1].dstSet = *descriptorSetPtr;
                              writeDescriptorSets[1].dstBinding = bufferDescriptorCount;
                              writeDescriptorSets[1].descriptorCount = 1;
                              writeDescriptorSets[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                              writeDescriptorSets[1].pBufferInfo = bufferDescriptors.data() + bufferDescriptorCount++;
                          }
                      }

                      if (uniforms_) {
                          bufferDescriptors[bufferDescriptorCount].buffer = uniforms_->handle();
                          bufferDescriptors[bufferDescriptorCount].offset = 0;
                          bufferDescriptors[bufferDescriptorCount].range = VK_WHOLE_SIZE;

                          writeDescriptorSets[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                          writeDescriptorSets[2].dstSet = *descriptorSetPtr;
                          writeDescriptorSets[2].dstBinding = bufferDescriptorCount;
                          writeDescriptorSets[2].descriptorCount = 1;
                          writeDescriptorSets[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                          writeDescriptorSets[2].pBufferInfo = bufferDescriptors.data() + bufferDescriptorCount++;
                      }

                      for (auto i = size_t{ 0 }, count = links.size(); i < count; i++) {
                          auto&& [idx, sampler, views, semantics] = links.get(i);

                          for (auto j = size_t{ 0 }; j < value_cast(RenderTargetOutputSemantic::COUNT); j++) {
                              auto semantic = semantics[j];

                              if (semantic == RenderTargetOutputSemantic::INVALID)
                                  continue;

                              imageDescriptors[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                              imageDescriptors[i].imageView = views[j];
                              imageDescriptors[i].sampler = sampler;

                              auto& writeDescriptorSet = writeDescriptorSets[bufferDescriptorCount + i];

                              writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                              writeDescriptorSet.dstSet = *descriptorSetPtr;
                              writeDescriptorSet.dstBinding = bufferDescriptorCount + i;
                              writeDescriptorSet.descriptorCount = 1;
                              writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                              writeDescriptorSet.pImageInfo = imageDescriptors.data() + imageDescriptorCount++;
                          }
                      }

                      vkUpdateDescriptorSets(device.handle(),
                                             bufferDescriptorCount + imageDescriptorCount,
                                             writeDescriptorSets.data(),
                                             0,
                                             nullptr);

                      flags |= mask;
                  } // descriptor set update

                  if (passIndex != 0) {
                      vkCmdNextSubpass(commandBuffer, VK_SUBPASS_CONTENTS_INLINE);
                  }

                  if (passType == compositor::PassType::SCENE) {
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
