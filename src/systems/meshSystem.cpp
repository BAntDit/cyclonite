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

auto MeshSystem::createGeometry(uint32_t vertexCount, uint32_t indexCount) -> std::shared_ptr<Geometry>
{
    auto [it, success] =
      geometries_.emplace(std::make_shared<Geometry>(vertexCount,
                                                     indexCount,
                                                     vertexBuffer_->alloc(vertexCount * sizeof(vertex_t)),
                                                     indexBuffer_->alloc(indexCount * sizeof(index_type_t))));

    assert(success);
    (void)success;

    return *it;
}

void MeshSystem::_addSubMesh(components::Mesh& mesh, std::shared_ptr<Geometry> const& geometry)
{
    auto* commands = reinterpret_cast<VkDrawIndexedIndirectCommand*>(commandBuffer_->ptr());

    auto firstIndex = geometry->firstIndex();
    auto baseVertex = geometry->baseVertex();

    auto idx = std::numeric_limits<size_t>::max();

    for (size_t i = 0; i < commandCount_; i++) {
        auto const& command = *(commands + i);

        if (command.firstIndex == firstIndex && command.vertexOffset == static_cast<int32_t>(baseVertex)) {
            idx = i;
            break;
        }
    }

    if (idx == std::numeric_limits<size_t>::max()) {
        if (commandDump_.empty()) {
            if (commandBuffer_->size() <= (commandCount_ + 1) * sizeof(VkDrawIndexedIndirectCommand)) {
                _reAllocCommandBuffer(std::max(size_t{ 1 }, commandCount_ * 2));
            }

            idx = commandCount_++;
        } else {
            idx = commandDump_.back();
            commandDump_.pop_back();
        }

        auto& command = *(commands + idx);

        command.indexCount = geometry->indexCount();
        command.instanceCount = 0;
        command.firstIndex = firstIndex;
        command.firstInstance = 0;
        command.vertexOffset = baseVertex;
    }

    mesh.subMeshes.emplace_back(idx, geometry);
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
