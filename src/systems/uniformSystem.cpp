//
// Created by bantdit on 3/21/20.
//

#include "uniformSystem.h"
#include "../vulkan/device.h"
#include <glm/gtc/type_ptr.hpp>

namespace cyclonite::systems {
void UniformSystem::init(vulkan::Device& device)
{
    vkDevice_ = device.handle();
    vkTransferQueue_ = device.hostTransferQueue();

    transferSemaphoreId_ = std::numeric_limits<size_t>::max();

    gpuUniforms_ = std::make_shared<vulkan::Buffer>(
      device,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
      sizeof(mat4) * 3,
      std::array{ device.hostTransferQueueFamilyIndex(), device.graphicsQueueFamilyIndex() });

    transferCommands_ = std::make_unique<vulkan::CommandBufferSet<vulkan::CommandPool, std::array<VkCommandBuffer, 1>>>(
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
                region.size = uniforms_->size();

                vkCmdCopyBuffer(transferCommandBuffer, uniforms_->handle(), gpuUniforms_->handle(), 1, &region);
            }

            if (auto result = vkEndCommandBuffer(transferCommandBuffer); result != VK_SUCCESS) {
                throw std::runtime_error("could not write uniforms transfer commands");
            }
        }));
}

void UniformSystem::setViewMatrix(mat4& viewMatrix)
{
    auto* ptr = reinterpret_cast<real*>(uniforms_->ptr());
    std::copy_n(glm::value_ptr(viewMatrix), 16, ptr);
}

void UniformSystem::setProjectionMatrix(mat4& projectionMatrix)
{
    auto* ptr = reinterpret_cast<real*>(uniforms_->ptr());
    std::copy_n(glm::value_ptr(projectionMatrix), 16, ptr + 16);
}

void UniformSystem::setViewProjectionMatrix(mat4& viewProjMatrix)
{
    auto* ptr = reinterpret_cast<real*>(uniforms_->ptr());
    std::copy_n(glm::value_ptr(viewProjMatrix), 16, ptr + 32);
}
}
