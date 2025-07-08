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
    auto initTask = [this,
                     &root,
                     swapChainLength,
                     initialCommandCapacity,
                     initialInstanceCapacity,
                     initialIndexCapacity,
                     initialVertexCapacity]() -> void {
        _init(root,
              swapChainLength,
              initialCommandCapacity,
              initialInstanceCapacity,
              initialIndexCapacity,
              initialVertexCapacity);
    };

    if (multithreading::Render::isInRenderThread()) {
        initTask();
    } else {
    }
}

void MeshSystem::_init(Root& root,
                       size_t swapChainLength,
                       size_t initialCommandCapacity,
                       size_t initialInstanceCapacity,
                       size_t initialIndexCapacity,
                       size_t initialVertexCapacity)
{
    // auto& device = root.device();

    /*commandBuffer_ = resourceManager_->template create<resources::Staging>(
      device, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, sizeof(VkDrawIndexedIndirectCommand) * initialCommandCapacity);

    gpuCommandBuffer_ = std::make_shared<vulkan::Buffer>(
      device,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
      sizeof(VkDrawIndexedIndirectCommand) * initialCommandCapacity,
      std::array{ uint32_t{0}, uint32_t{0} // device.hostTransferQueueFamilyIndex(), device.graphicsQueueFamilyIndex()
          });

    instancedDataBuffer_ = resourceManager_->template create<resources::Staging>(
      device, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, sizeof(instanced_data_t) * initialInstanceCapacity);

    indexBuffer_ = resourceManager_->template create<resources::Staging>(
      device, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, sizeof(index_type_t) * initialIndexCapacity);

    vertexBuffer_ = resourceManager_->template create<resources::Staging>(
      device, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, sizeof(vertex_t) * initialVertexCapacity);

    gpuInstancedDataBuffer_ = std::make_shared<vulkan::Buffer>(
      device,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
      sizeof(instanced_data_t) * initialInstanceCapacity,
      std::array{ uint32_t{0}, uint32_t{0} // device.hostTransferQueueFamilyIndex(), device.graphicsQueueFamilyIndex()
          });

    gpuIndexBuffer_ = std::make_shared<vulkan::Buffer>(
      device,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
      sizeof(index_type_t) * initialIndexCapacity,
      std::array{ uint32_t{0}, uint32_t{0} // device.hostTransferQueueFamilyIndex(), device.graphicsQueueFamilyIndex()
          });

    gpuVertexBuffer_ = std::make_unique<vulkan::Buffer>(
      device,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
      sizeof(vertex_t) * initialVertexCapacity,
      std::array{ uint32_t{0}, uint32_t{0} // device.hostTransferQueueFamilyIndex(), device.graphicsQueueFamilyIndex()
          });

    transferSemaphores_.reserve(swapChainLength);*/

    /*transferCommands_ = std::make_unique<vulkan::CommandBufferSet<vulkan::CommandPool, std::array<VkCommandBuffer,
      3>>>( device.commandPool().allocCommandBuffers( vulkan::CommandBufferSet<vulkan::CommandPool,
      std::array<VkCommandBuffer, 3>>{ device.hostTransferQueueFamilyIndex(),
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
                    auto& instanceDataBuffer =
                      resourceManager_->get(instancedDataBuffer_).template as<resources::Staging>();

                    VkBufferCopy region = {};
                    region.srcOffset = 0;
                    region.dstOffset = 0;
                    region.size = instanceDataBuffer.size();

                    vkCmdCopyBuffer(
                      transferCommands, instanceDataBuffer.handle(), gpuInstancedDataBuffer_->handle(), 1, &region);
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
                    auto& commandBuffer = resourceManager_->get(commandBuffer_).template as<resources::Staging>();

                    VkBufferCopy region = {};
                    region.srcOffset = 0;
                    region.dstOffset = 0;
                    region.size = commandBuffer.size();

                    vkCmdCopyBuffer(transferCommands, commandBuffer.handle(), gpuCommandBuffer_->handle(), 1, &region);
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
                    auto& vertexBuffer = resourceManager_->get(vertexBuffer_).template as<resources::Staging>();

                    VkBufferCopy region = {};
                    region.srcOffset = 0;
                    region.dstOffset = 0;
                    region.size = vertexBuffer.size();

                    vkCmdCopyBuffer(transferCommands, vertexBuffer.handle(), gpuVertexBuffer_->handle(), 1, &region);
                }

                {
                    auto& indexBuffer = resourceManager_->get(indexBuffer_).template as<resources::Staging>();
                    VkBufferCopy region = {};
                    region.srcOffset = 0;
                    region.dstOffset = 0;
                    region.size = indexBuffer.size();

                    vkCmdCopyBuffer(transferCommands, indexBuffer.handle(), gpuIndexBuffer_->handle(), 1, &region);
                }

                if (auto result = vkEndCommandBuffer(transferCommands); result != VK_SUCCESS) {
                    throw std::runtime_error("could not write uniforms transfer commands");
                }
            }
        }));*/

    verticesUpdateRequired_ = false;
}

void MeshSystem::requestVertexDeviceBufferUpdate()
{
    verticesUpdateRequired_ = true;
}

auto MeshSystem::createGeometry(uint32_t vertexCount, uint32_t indexCount) -> uint64_t
{
    return static_cast<uint64_t>(0);
}

void MeshSystem::_addSubMesh(components::SubMesh& subMesh, uint64_t geometryId) {}

auto MeshSystem::_getDumpCommandIndex() -> size_t
{
    return 0;
}

void MeshSystem::_reAllocCommandBuffer(size_t size)
{
    (void)size;
    throw std::runtime_error("not implemented yet");
}
}
