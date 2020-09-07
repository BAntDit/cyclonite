//
// Created by bantdit on 2/7/20.
//

#include "meshSystem.h"
#include "vulkan/device.h"

namespace cyclonite::systems {
void MeshSystem::init(vulkan::Device& device,
                      size_t initialCommandCapacity,
                      size_t initialInstanceCapacity,
                      size_t initialIndexCapacity,
                      size_t initialVertexCapacity)
{
    devicePtr_ = &device;

    commandBuffer_ = std::make_unique<vulkan::Staging>(
      device, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, sizeof(VkDrawIndexedIndirectCommand) * initialCommandCapacity);

    gpuCommandBuffer_ = std::make_shared<vulkan::Buffer>(
      device,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
      sizeof(VkDrawIndexedIndirectCommand) * initialCommandCapacity,
      std::array{ device.hostTransferQueueFamilyIndex(), device.graphicsQueueFamilyIndex() });

    commandCount_ = 0;

    instancedDataBuffer_ = std::make_unique<vulkan::Staging>(
      device, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, sizeof(instanced_data_t) * initialInstanceCapacity);

    indexBuffer_ = std::make_unique<vulkan::Staging>(
      device, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, sizeof(index_type_t) * initialIndexCapacity);

    vertexBuffer_ = std::make_unique<vulkan::Staging>(
      device, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, sizeof(vertex_t) * initialVertexCapacity);

    gpuInstancedDataBuffer_ = std::make_shared<vulkan::Buffer>(
      device,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
      sizeof(instanced_data_t) * initialInstanceCapacity,
      std::array{ device.hostTransferQueueFamilyIndex(), device.graphicsQueueFamilyIndex() });

    gpuIndexBuffer_ = std::make_shared<vulkan::Buffer>(
      device,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
      sizeof(index_type_t) * initialIndexCapacity,
      std::array{ device.hostTransferQueueFamilyIndex(), device.graphicsQueueFamilyIndex() });

    gpuVertexBuffer_ = std::make_unique<vulkan::Buffer>(
      device,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      sizeof(vertex_t) * initialVertexCapacity,
      std::array{ device.hostTransferQueueFamilyIndex(), device.graphicsQueueFamilyIndex() });
}

void MeshSystem::_addSubMesh(components::Mesh& mesh, uint32_t indexCount, uint32_t vertexCount)
{
    auto indexMemory = indexBuffer_->alloc(indexCount * sizeof(index_type_t));
    auto vertexMemory = vertexBuffer_->alloc(vertexCount * sizeof(vertex_t));

    uint32_t firstIndex = indexMemory.offset() / sizeof(index_type_t);
    uint32_t vertexOffset = vertexMemory.offset() / sizeof(vertex_t);

    auto* commands = reinterpret_cast<VkDrawIndexedIndirectCommand*>(commandBuffer_->ptr());

    auto lastCommand = commands + commandCount_ + 1;

    auto* command = std::find_if(commands, lastCommand, [&](auto&& cmd) -> bool {
        return cmd.firstIndex == firstIndex && cmd.vertexOffset == vertexOffset;
    });

    auto idx = command - commands;

    if (command == lastCommand) {
        lastCommand->indexCount = indexCount;
        lastCommand->instanceCount = 1;
        lastCommand->firstIndex = firstIndex;
        lastCommand->vertexOffset = vertexOffset;
        lastCommand->firstInstance = 0;

        commandCount_++;
    } else {
        command->instanceCount++;
    }

    mesh.subMeshes.emplace_back(std::move(indexMemory), std::move(vertexMemory), idx);
}

void MeshSystem::_reAllocCommandBuffer(size_t size)
{
    auto commandBuffer = std::make_unique<vulkan::Staging>(*devicePtr_, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, size);

    std::copy_n(reinterpret_cast<VkDrawIndexedIndirectCommand*>(commandBuffer_->ptr()),
                commandCount_,
                reinterpret_cast<VkDrawIndexedIndirectCommand*>(commandBuffer->ptr()));

    commandBuffer_ = std::move(commandBuffer);
}
}
