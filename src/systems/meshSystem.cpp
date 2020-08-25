//
// Created by bantdit on 2/7/20.
//

#include "meshSystem.h"
#include "vulkan/device.h"

namespace cyclonite::systems {
void MeshSystem::init(vulkan::Device& device,
                      size_t initialCommandCount,
                      size_t initialInstanceCount,
                      size_t initialIndexCount,
                      size_t initialVertexCount)
{
    vkDevice_ = device.handle();

    commandBuffer_ = std::make_unique<vulkan::Staging>(
      device, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, sizeof(VkDrawIndexedIndirectCommand) * initialCommandCount);

    instancedDataBuffer_ = std::make_unique<vulkan::Staging>(
      device, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, sizeof(instanced_data_t) * initialInstanceCount);

    indexBuffer_ = std::make_unique<vulkan::Staging>(
      device, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, sizeof(index_type_t) * initialIndexCount);

    vertexBuffer_ = std::make_unique<vulkan::Staging>(
      device, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, sizeof(vertex_t) * initialVertexCount);

    gpuCommandBuffer_ = std::make_shared<vulkan::Buffer>(
      device,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
      sizeof(VkDrawIndexedIndirectCommand) * initialCommandCount,
      std::array{ device.hostTransferQueueFamilyIndex(), device.graphicsQueueFamilyIndex() });

    gpuInstancedDataBuffer_ = std::make_shared<vulkan::Buffer>(
      device,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
      sizeof(instanced_data_t) * initialInstanceCount,
      std::array{ device.hostTransferQueueFamilyIndex(), device.graphicsQueueFamilyIndex() });

    gpuIndexBuffer_ = std::make_shared<vulkan::Buffer>(
      device,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
      sizeof(index_type_t) * initialIndexCount,
      std::array{ device.hostTransferQueueFamilyIndex(), device.graphicsQueueFamilyIndex() });

    gpuVertexBuffer_ = std::make_unique<vulkan::Buffer>(
      device,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      sizeof(vertex_t) * initialVertexCount,
      std::array{ device.hostTransferQueueFamilyIndex(), device.graphicsQueueFamilyIndex() });
}
}
