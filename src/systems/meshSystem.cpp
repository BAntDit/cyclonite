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

    gpuCommandBuffer_ = std::make_unique<vulkan::Buffer>(
      device,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
      sizeof(VkDrawIndexedIndirectCommand),
      std::array{ device.hostTransferQueueFamilyIndex(), device.graphicsQueueFamilyIndex() });

    transferCommands_ = std::make_unique<vulkan::CommandBufferSet<std::array<VkCommandBuffer, 1>>>(
      device.commandPool().allocCommandBuffers(
        vulkan::CommandBufferSet<std::array<VkCommandBuffer, 1>>{ device.hostTransferQueueFamilyIndex(),
                                                                  VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                                                                  std::array<VkCommandBuffer, 1>{} },
        [&, this](std::array<VkCommandBuffer, 1>& transferCommandBuffer) -> void {
            VkBufferCopy region = {};
            region.srcOffset = 0;
            region.dstOffset = 0;
            region.size = commandBuffer_->size();

            vkCmdCopyBuffer(
              transferCommandBuffer[0], commandBuffer_->handle(), gpuCommandBuffer_->handle(), 1, &region);
        }));

    // TODO:: prepare indices of the cube
}
}
