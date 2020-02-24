//
// Created by bantdit on 2/24/20.
//

#include "cameraSystem.h"
#include "vulkan/device.h"

namespace cyclonite::systems
{
void CameraSystem::init(vulkan::Device& device)
{
    uniforms_ =
      std::make_unique<vulkan::Staging>(device, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, sizeof(mat4) * 3);

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
        throw std::runtime_error("could not create pass end synchronization semaphore");
    }
}
}
