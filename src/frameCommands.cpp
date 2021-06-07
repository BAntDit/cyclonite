//
// Created by anton on 4/18/21.
//

#include "frameCommands.h"
#include "compositor/links.h"
#include "vulkan/device.h"

namespace cyclonite {
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

FrameCommands::FrameCommands()
  : indices_{ nullptr }
  , vertices_{ nullptr }
  , instances_{ nullptr }
  , commands_{ nullptr }
  , uniforms_{ nullptr }
  , graphicsCommands_{ nullptr }
{}

void FrameCommands::update(vulkan::Device& device,
                           compositor::Links const& links,
                           compositor::PassIterator const& begin,
                           compositor::PassIterator const& end)
{
    if (!graphicsCommands_) {
        graphicsCommands_ = std::make_unique<graphics_queue_commands_t>(device.commandPool().allocCommandBuffers(
          vulkan::CommandBufferSet<vulkan::CommandPool, std::array<VkCommandBuffer, 1>>{
            device.graphicsQueueFamilyIndex(),
            VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            std::array<VkCommandBuffer, 1>{} },
          [&](auto&& graphicsCommands) -> void {
              auto [commandBuffer] = graphicsCommands;

              // all passes:
              auto passIndex = size_t{0};

              for (auto it = begin; it != end; it++) {
                  auto [passType,
                        descriptorPool,
                        descriptorSetLayout,
                        pipelineLayout,
                        pipeline,
                        descriptorSetPtr,
                        flagsPtr] = *it;

                  auto mask = static_cast<uint8_t>(1 << (passIndex % CHAR_BIT));
                  bool isExpired = (static_cast<uint8_t>(*flagsPtr) & mask) != 0;

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
                          imageDescriptors[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                          imageDescriptors[i].imageView = links.get(i).handle();
                          imageDescriptors[i].sampler = VK_NULL_HANDLE;

                          auto& writeDescriptorSet = writeDescriptorSets[bufferDescriptorCount + i];

                          writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                          writeDescriptorSet.dstSet = *descriptorSetPtr;
                          writeDescriptorSet.dstBinding = bufferDescriptorCount + i;
                          writeDescriptorSet.descriptorCount = 1;
                          writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
                          writeDescriptorSet.pImageInfo = imageDescriptors.data() + imageDescriptorCount++;
                      }

                      vkUpdateDescriptorSets(device.handle(),
                                             bufferDescriptorCount + imageDescriptorCount,
                                             writeDescriptorSets.data(),
                                             0,
                                             nullptr);

                      auto flags = static_cast<uint8_t>(*flagsPtr) | mask;
                      *flagsPtr = static_cast<std::byte>(flags);
                  } // descriptor set update

                  VkCommandBufferBeginInfo commandBufferBeginInfo = {};
                  commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

                  if (vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo) != VK_SUCCESS) {
                      throw std::runtime_error("could not begin recording command buffer!");
                  }

                  // TODO:: make it for all passes (subpasses)

                  if (auto result = vkEndCommandBuffer(commandBuffer); result != VK_SUCCESS) {
                      throw std::runtime_error("could not record command buffer!");
                  }

                  passIndex++;
              } // pass cycle end

          }));
    } // graphic commands
}
}
