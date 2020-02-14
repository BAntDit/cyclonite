//
// Created by bantdit on 2/7/20.
//

#include "meshSystem.h"
#include "vulkan/device.h"

namespace cyclonite::systems {
void MeshSystem::init(vulkan::Device& device)
{
    commandBuffer_ =
      std::make_unique<vulkan::Staging>(device, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, sizeof(VkDrawIndexedIndirectCommand));

    indicesBuffer_ = std::make_unique<vulkan::Staging>(device, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, sizeof(uint32_t) * 36);

    transformBuffer_ =
      std::make_unique<vulkan::Staging>(device, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, sizeof(mat3x4) * 100);

    gpuCommandBuffer_ = std::make_unique<vulkan::Buffer>(
      device,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
      sizeof(VkDrawIndexedIndirectCommand),
      std::array{ device.hostTransferQueueFamilyIndex(), device.graphicsQueueFamilyIndex() });

    gpuIndicesBuffer_ = std::make_unique<vulkan::Buffer>(
      device,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
      sizeof(uint32_t) * 36,
      std::array{ device.hostTransferQueueFamilyIndex(), device.graphicsQueueFamilyIndex() });

    gpuTransformBuffer_ = std::make_unique<vulkan::Buffer>(
      device,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
      sizeof(mat3x4) * 100,
      std::array{ device.hostTransferQueueFamilyIndex(), device.graphicsQueueFamilyIndex() });

    {
        std::array<uint32_t, 36> indices = { // front
                                             0,
                                             1,
                                             2,
                                             2,
                                             3,
                                             0,
                                             // right
                                             1,
                                             5,
                                             6,
                                             6,
                                             2,
                                             1,
                                             // back
                                             7,
                                             6,
                                             5,
                                             5,
                                             4,
                                             7,
                                             // left
                                             4,
                                             0,
                                             3,
                                             3,
                                             7,
                                             4,
                                             // bottom
                                             4,
                                             5,
                                             1,
                                             1,
                                             0,
                                             4,
                                             // top
                                             3,
                                             2,
                                             6,
                                             6,
                                             7,
                                             3
        };

        std::copy_n(indices.begin(), 36, reinterpret_cast<uint32_t*>(indicesBuffer_->ptr()));
    }

    vertexInputTransfer_ = std::make_unique<vulkan::CommandBufferSet<std::array<VkCommandBuffer, 1>>>(
      device.commandPool().allocCommandBuffers(
        vulkan::CommandBufferSet<std::array<VkCommandBuffer, 1>>{ device.hostTransferQueueFamilyIndex(),
                                                                  VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                                                                  std::array<VkCommandBuffer, 1>{} },
        [&, this](std::array<VkCommandBuffer, 1>& transferCommandBuffers) -> void {
            auto& transferCommandBuffer = transferCommandBuffers[0];

            VkBufferCopy region = {};
            region.srcOffset = 0;
            region.dstOffset = 0;
            region.size = indicesBuffer_->size();

            vkCmdCopyBuffer(transferCommandBuffer, indicesBuffer_->handle(), gpuIndicesBuffer_->handle(), 1, &region);
        }));

    renderCommandsTransfer_ = std::make_unique<vulkan::CommandBufferSet<std::array<VkCommandBuffer, 1>>>(
      device.commandPool().allocCommandBuffers(
        vulkan::CommandBufferSet<std::array<VkCommandBuffer, 1>>{ device.hostTransferQueueFamilyIndex(),
                                                                  VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                                                                  std::array<VkCommandBuffer, 1>{} },
        [&, this](std::array<VkCommandBuffer, 1>& transferCommandBuffers) -> void {
            auto& transferCommandBuffer = transferCommandBuffers[0];

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
        }));
}
}
