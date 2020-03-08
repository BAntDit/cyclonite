//
// Created by bantdit on 2/24/20.
//

#include "cameraSystem.h"
#include "vulkan/device.h"

namespace cyclonite::systems {
void CameraSystem::init(vulkan::Device& device)
{
    uniforms_ = std::make_unique<vulkan::Staging>(device, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, sizeof(mat4) * 3);

    gpuUniforms_ = std::make_unique<vulkan::Buffer>(
      device,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
      sizeof(mat4) * 3,
      std::array{ device.hostTransferQueueFamilyIndex(), device.graphicsQueueFamilyIndex() });

    transferSemaphore_ = vulkan::Handle<VkSemaphore>{ device.handle(), vkDestroySemaphore };

    VkSemaphoreCreateInfo semaphoreCreateInfo = {};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    if (auto result = vkCreateSemaphore(device.handle(), &semaphoreCreateInfo, nullptr, &transferSemaphore_);
        result != VK_SUCCESS) {
        throw std::runtime_error("could not create uniforms transfer synchronization semaphore");
    }

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

              if (auto result = vkBeginCommandBuffer(transferCommandBuffer, &beginInfo); result != VK_SUCCESS) {
                  throw std::runtime_error("could not begin to write uniforms transfer commands");
              }

              {
                  VkBufferCopy region = {};
                  region.srcOffset = 0;
                  region.dstOffset = 0;
                  region.size = uniforms_->size();

                  vkCmdCopyBuffer(transferCommandBuffer, uniforms_->handle(), gpuUniforms_->handle(), 1, &region);
              }

              if (auto result = vkEndCommandBuffer(transferCommandBuffer); result != VK_SUCCESS) {
                  throw std::runtime_error("could not write uniforms transfer commands");
              }
          }));
}

auto CameraSystem::persistentTransferCommands() const -> std::shared_ptr<transfer_commands_t> const&
{
    return persistentTransfer_;
}
}
