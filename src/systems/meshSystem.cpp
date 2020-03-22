//
// Created by bantdit on 2/7/20.
//

#include "meshSystem.h"
#include "vulkan/device.h"

namespace cyclonite::systems {
void MeshSystem::init(vulkan::Device& device)
{
    vkDevice_ = device.handle();

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

    constexpr size_t elementCount = 36;

    indicesBuffer_ =
      std::make_unique<vulkan::Staging>(device, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, sizeof(uint32_t) * elementCount);

    gpuIndicesBuffer_ = std::make_shared<vulkan::Buffer>(
      device,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
      sizeof(uint32_t) * elementCount,
      std::array{ device.hostTransferQueueFamilyIndex(), device.graphicsQueueFamilyIndex() });

    {
        std::array<uint32_t, elementCount> indices = { 0,  1,  2,  2,  3,  0,  4,  5,  6,  6,  7,  4,
                                                       8,  9,  10, 10, 11, 8,  12, 13, 14, 14, 15, 12,
                                                       16, 17, 18, 18, 19, 16, 22, 21, 20, 20, 23, 22 };

        std::copy_n(indices.begin(), indices.size(), reinterpret_cast<uint32_t*>(indicesBuffer_->ptr()));
    }
}
}
