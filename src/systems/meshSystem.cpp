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

    gpuCommandBuffer_ =
      std::make_unique<vulkan::Buffer>(device,
                                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                       VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                                       sizeof(VkDrawIndexedIndirectCommand),
                                       std::array{ device.graphicsQueueFamilyIndex() });
}
}
