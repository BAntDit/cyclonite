//
// Created by bantdit on 2/7/20.
//

#include "meshSystem.h"
#include "resources/geometry.h"
#include "root.h"

namespace cyclonite::systems {
void MeshSystem::init(Root& root,
                      size_t swapChainLength,
                      size_t initialCommandCapacity,
                      size_t initialInstanceCapacity,
                      size_t initialIndexCapacity,
                      size_t initialVertexCapacity)
{
    auto& device = root.device();

    devicePtr_ = &device;
    resourceManager_ = &root.resourceManager();

    commandCount_ = 0;

    vkTransferQueue_ = device.hostTransferQueue();
    vkGraphicQueue_ = device.graphicsQueue();

    commands_.reserve(initialCommandCapacity);

    commandBuffer_ = std::make_unique<vulkan::Staging>(
      device, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, sizeof(VkDrawIndexedIndirectCommand) * initialCommandCapacity);

    gpuCommandBuffer_ = std::make_shared<vulkan::Buffer>(
      device,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
      sizeof(VkDrawIndexedIndirectCommand) * initialCommandCapacity,
      std::array{ device.hostTransferQueueFamilyIndex(), device.graphicsQueueFamilyIndex() });

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
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
      sizeof(vertex_t) * initialVertexCapacity,
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

    transferCommands_ = std::make_unique<vulkan::CommandBufferSet<vulkan::CommandPool, std::array<VkCommandBuffer, 3>>>(
      device.commandPool().allocCommandBuffers(
        vulkan::CommandBufferSet<vulkan::CommandPool, std::array<VkCommandBuffer, 3>>{
          device.hostTransferQueueFamilyIndex(),
          VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
          std::array<VkCommandBuffer, 3>{} },
        [&, this](auto& transferCommandBuffers) -> void {
            VkCommandBufferBeginInfo beginInfo = {};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

            { // instances
                auto& transferCommands = transferCommandBuffers[0];

                if (auto result = vkBeginCommandBuffer(transferCommands, &beginInfo); result != VK_SUCCESS) {
                    throw std::runtime_error("could not begin to write uniforms transfer commands");
                }

                {
                    VkBufferCopy region = {};
                    region.srcOffset = 0;
                    region.dstOffset = 0;
                    region.size = instancedDataBuffer_->size();

                    vkCmdCopyBuffer(
                      transferCommands, instancedDataBuffer_->handle(), gpuInstancedDataBuffer_->handle(), 1, &region);
                }

                if (auto result = vkEndCommandBuffer(transferCommands); result != VK_SUCCESS) {
                    throw std::runtime_error("could not write uniforms transfer commands");
                }
            }

            { // commands
                auto& transferCommands = transferCommandBuffers[1];

                if (auto result = vkBeginCommandBuffer(transferCommands, &beginInfo); result != VK_SUCCESS) {
                    throw std::runtime_error("could not begin to write uniforms transfer commands");
                }

                {
                    VkBufferCopy region = {};
                    region.srcOffset = 0;
                    region.dstOffset = 0;
                    region.size = commandBuffer_->size();

                    vkCmdCopyBuffer(
                      transferCommands, commandBuffer_->handle(), gpuCommandBuffer_->handle(), 1, &region);
                }

                if (auto result = vkEndCommandBuffer(transferCommands); result != VK_SUCCESS) {
                    throw std::runtime_error("could not write uniforms transfer commands");
                }
            }

            { // vertices
                auto& transferCommands = transferCommandBuffers[2];

                if (auto result = vkBeginCommandBuffer(transferCommands, &beginInfo); result != VK_SUCCESS) {
                    throw std::runtime_error("could not begin to write uniforms transfer commands");
                }

                {
                    VkBufferCopy region = {};
                    region.srcOffset = 0;
                    region.dstOffset = 0;
                    region.size = vertexBuffer_->size();

                    vkCmdCopyBuffer(transferCommands, vertexBuffer_->handle(), gpuVertexBuffer_->handle(), 1, &region);
                }

                {
                    VkBufferCopy region = {};
                    region.srcOffset = 0;
                    region.dstOffset = 0;
                    region.size = indexBuffer_->size();

                    vkCmdCopyBuffer(transferCommands, indexBuffer_->handle(), gpuIndexBuffer_->handle(), 1, &region);
                }

                if (auto result = vkEndCommandBuffer(transferCommands); result != VK_SUCCESS) {
                    throw std::runtime_error("could not write uniforms transfer commands");
                }
            }
        }));

    verticesUpdateRequired_ = false;
}

void MeshSystem::requestVertexDeviceBufferUpdate()
{
    verticesUpdateRequired_ = true;
}

auto MeshSystem::createGeometry(uint32_t vertexCount, uint32_t indexCount) -> uint64_t
{
    // TODO:: move outside after stagings
    auto id =
      resourceManager_->template create<resources::Geometry>(vertexCount,
                                                             indexCount,
                                                             vertexBuffer_->alloc(vertexCount * sizeof(vertex_t)),
                                                             indexBuffer_->alloc(indexCount * sizeof(index_type_t)));

    return static_cast<uint64_t>(id);
}

void MeshSystem::_addSubMesh(components::SubMesh& subMesh, uint64_t geometryId)
{
    auto& geometry = resourceManager_->get(resources::Resource::Id{ geometryId }).template as<resources::Geometry>();

    auto firstIndex = geometry.firstIndex();
    auto baseVertex = geometry.baseVertex();

    auto idx = std::numeric_limits<size_t>::max();

    {
        auto i =
          std::distance(commands_.cbegin(), std::find_if(commands_.cbegin(), commands_.cend(), [=](auto&& cmd) -> bool {
                            return cmd.firstIndex == firstIndex && cmd.vertexOffset == static_cast<int32_t>(baseVertex);
                        }));

        assert(i >= 0);

        idx = i;
    }

    assert(idx <= commands_.size());

    if (idx == commands_.size()) {
        idx =
          commandDump_.empty() ? (commands_.emplace_back(VkDrawIndexedIndirectCommand{}), idx) : _getDumpCommandIndex();

        auto&& command = commands_[idx];

        command.indexCount = geometry.indexCount();
        command.instanceCount = 0;
        command.firstIndex = firstIndex;
        command.firstInstance = 0;
        command.vertexOffset = baseVertex;
    }

    // TODO:: realloc buffers and transfer commands if commands_ size > commandBuffer_->size()

    subMesh.commandIndex = idx;
    subMesh.geometryId = geometryId;
}

auto MeshSystem::_getDumpCommandIndex() -> size_t
{
    auto idx = commandDump_.back();
    commandDump_.pop_back();
    return idx;
}

void MeshSystem::_reAllocCommandBuffer(size_t size)
{
    (void)size;
    throw std::runtime_error("not implemented yet");
}
}
