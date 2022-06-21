//
// Created by bantdit on 3/21/20.
//

#include "uniformSystem.h"
#include "resources/resourceManager.h"
#include "vulkan/device.h"
#include <glm/gtc/type_ptr.hpp>

namespace cyclonite::systems {
void UniformSystem::init(resources::ResourceManager& resourceManager, vulkan::Device& device, size_t swapChainLength)
{
    devicePtr_ = &device;
    resourceManager_ = &resourceManager;
    vkTransferQueue_ = device.hostTransferQueue();

    uniforms_ =
      resourceManager_->template create<resources::Staging>(device, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, sizeof(mat4) * 3);

    gpuUniforms_ = std::make_shared<vulkan::Buffer>(
      device,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
      sizeof(mat4) * 3,
      std::array{ device.hostTransferQueueFamilyIndex(), device.graphicsQueueFamilyIndex() });

    transferSemaphores_.reserve(swapChainLength);

    VkSemaphoreCreateInfo semaphoreCreateInfo = {};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    for (size_t i = 0; i < swapChainLength; i++) {
        if (auto result = vkCreateSemaphore(device.handle(),
                                            &semaphoreCreateInfo,
                                            nullptr,
                                            &transferSemaphores_.emplace_back(device.handle(), vkDestroySemaphore));
            result != VK_SUCCESS) {
            throw std::runtime_error("could not create transfer synchronization semaphore");
        }
    }

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
                auto& staging = resourceManager_->get(uniforms_).template as<resources::Staging>();
                VkBufferCopy region = {};
                region.srcOffset = 0;
                region.dstOffset = 0;
                region.size = staging.size();

                vkCmdCopyBuffer(transferCommandBuffer, staging.handle(), gpuUniforms_->handle(), 1, &region);
            }

            if (auto result = vkEndCommandBuffer(transferCommandBuffer); result != VK_SUCCESS) {
                throw std::runtime_error("could not write uniforms transfer commands");
            }
        }));
}

void UniformSystem::setViewMatrix(mat4& viewMatrix)
{
    auto& staging = resourceManager_->get(uniforms_).template as<resources::Staging>();
    auto* ptr = reinterpret_cast<real*>(staging.ptr());
    std::copy_n(glm::value_ptr(viewMatrix), 16, ptr);
}

void UniformSystem::setProjectionMatrix(mat4& projectionMatrix)
{
    auto& staging = resourceManager_->get(uniforms_).template as<resources::Staging>();
    auto* ptr = reinterpret_cast<real*>(staging.ptr());
    std::copy_n(glm::value_ptr(projectionMatrix), 16, ptr + 16);
}

void UniformSystem::setViewProjectionMatrix(mat4& viewProjMatrix)
{
    auto& staging = resourceManager_->get(uniforms_).template as<resources::Staging>();
    auto* ptr = reinterpret_cast<real*>(staging.ptr());
    std::copy_n(glm::value_ptr(viewProjMatrix), 16, ptr + 32);
}

auto UniformSystem::uniforms() const -> resources::Staging const&
{
    return resourceManager_->get(uniforms_).template as<resources::Staging>();
}

auto UniformSystem::uniforms() -> resources::Staging&
{
    return resourceManager_->get(uniforms_).template as<resources::Staging>();
}

}
