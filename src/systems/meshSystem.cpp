//
// Created by bantdit on 2/7/20.
//

#include "meshSystem.h"
#include "vulkan/device.h"

namespace cyclonite::systems {
void MeshSystem::init(vulkan::Device& device)
{
    vkDevice_ = device.handle();

    graphicsVersion_ = std::numeric_limits<uint32_t>::max();

    // commands:
    // one command just for now
    commandBuffer_ =
      std::make_unique<vulkan::Staging>(device, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, sizeof(VkDrawIndexedIndirectCommand));

    gpuCommandBuffer_ = std::make_shared<vulkan::Buffer>(
      device,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
      sizeof(VkDrawIndexedIndirectCommand),
      std::array{ device.hostTransferQueueFamilyIndex(), device.graphicsQueueFamilyIndex() });
    //

    // transforms
    transformBuffer_ =
      std::make_unique<vulkan::Staging>(device, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, sizeof(mat3x4) * 100);

    gpuTransformBuffer_ = std::make_shared<vulkan::Buffer>(
      device,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
      sizeof(mat3x4) * 100,
      std::array{ device.hostTransferQueueFamilyIndex(), device.graphicsQueueFamilyIndex() });
    //

    persistentTransfer_ =
      std::make_shared<vulkan::CommandBufferSet<vulkan::CommandPool, std::array<VkCommandBuffer, 1>>>(
        device.commandPool().allocCommandBuffers(
          vulkan::CommandBufferSet<vulkan::CommandPool, std::array<VkCommandBuffer, 1>>{
            device.hostTransferQueueFamilyIndex(),
            VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            std::array<VkCommandBuffer, 1>{} },
          [&, this](std::array<VkCommandBuffer, 1>& transferCommandBuffers) -> void {
              auto& transferCommandBuffer = transferCommandBuffers[0];

              VkCommandBufferBeginInfo beginInfo = {};
              beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
              beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

              if (auto result = vkBeginCommandBuffer(transferCommandBuffer, &beginInfo); result != VK_SUCCESS) {
                  throw std::runtime_error("could not begin to write uniforms transfer commands");
              }

              {
                  VkBufferCopy region = {};
                  region.srcOffset = 0;
                  region.dstOffset = 0;
                  region.size = transformBuffer_->size();

                  vkCmdCopyBuffer(
                    transferCommandBuffer, transformBuffer_->handle(), gpuTransformBuffer_->handle(), 1, &region);
              }

              {
                  VkBufferCopy region = {};
                  region.srcOffset = 0;
                  region.dstOffset = 0;
                  region.size = commandBuffer_->size();

                  vkCmdCopyBuffer(
                    transferCommandBuffer, commandBuffer_->handle(), gpuCommandBuffer_->handle(), 1, &region);
              }

              if (auto result = vkEndCommandBuffer(transferCommandBuffer); result != VK_SUCCESS) {
                  throw std::runtime_error("could not write uniforms transfer commands");
              }
          }));

    transferSemaphore_ = vulkan::Handle<VkSemaphore>{ vkDevice_, vkDestroySemaphore };

    VkSemaphoreCreateInfo semaphoreCreateInfo = {};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    if (auto result = vkCreateSemaphore(device.handle(), &semaphoreCreateInfo, nullptr, &transferSemaphore_);
        result != VK_SUCCESS) {
        throw std::runtime_error("could not create transfer synchronization semaphore");
    }
    //

    // transient transfer
    indicesBuffer_ = std::make_unique<vulkan::Staging>(device, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, sizeof(uint32_t) * 6);

    gpuIndicesBuffer_ = std::make_shared<vulkan::Buffer>(
      device,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
      sizeof(uint32_t) * 6,
      std::array{ device.hostTransferQueueFamilyIndex(), device.graphicsQueueFamilyIndex() });

    {
        std::array<uint32_t, 6> indices = { 0, 1, 2, 2, 3, 0 };

        std::copy_n(indices.begin(), indices.size(), reinterpret_cast<uint32_t*>(indicesBuffer_->ptr()));
    }

    transientTransfer_ =
      std::make_unique<vulkan::CommandBufferSet<vulkan::CommandPool, std::array<VkCommandBuffer, 1>>>(
        device.commandPool().allocCommandBuffers(
          vulkan::CommandBufferSet<vulkan::CommandPool, std::array<VkCommandBuffer, 1>>{
            device.hostTransferQueueFamilyIndex(),
            VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
            std::array<VkCommandBuffer, 1>{} },
          [&, this](std::array<VkCommandBuffer, 1>& transferCommandBuffers) -> void {
              auto& transferCommandBuffer = transferCommandBuffers[0];

              VkCommandBufferBeginInfo beginInfo = {};
              beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
              beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

              if (auto result = vkBeginCommandBuffer(transferCommandBuffer, &beginInfo); result != VK_SUCCESS) {
                  throw std::runtime_error("could not begin to write uniforms transfer commands");
              }

              VkBufferCopy region = {};
              region.srcOffset = 0;
              region.dstOffset = 0;
              region.size = indicesBuffer_->size();

              vkCmdCopyBuffer(transferCommandBuffer, indicesBuffer_->handle(), gpuIndicesBuffer_->handle(), 1, &region);

              if (auto result = vkEndCommandBuffer(transferCommandBuffer); result != VK_SUCCESS) {
                  throw std::runtime_error("could not write uniforms transfer commands");
              }
          }));
}

auto MeshSystem::persistentTransferCommands() const -> std::shared_ptr<transfer_commands_t> const&
{
    return persistentTransfer_;
}

auto MeshSystem::transientTransferCommands() const -> std::unique_ptr<transfer_commands_t> const&
{
    return transientTransfer_;
}
}
