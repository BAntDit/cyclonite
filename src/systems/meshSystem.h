//
// Created by bantdit on 2/6/20.
//

#ifndef CYCLONITE_MESHSYSTEM_H
#define CYCLONITE_MESHSYSTEM_H

#include "components/mesh.h"
#include "components/transform.h"
#include "resources/resourceManager.h"
#include "resources/staging.h"
#include "transformSystem.h"
#include "vulkan/buffer.h"
#include "vulkan/commandBufferSet.h"
#include "vulkan/commandPool.h"
#include "vulkan/device.h"
#include <easy-mp/enum.h>
#include <enttx/enttx.h>
#include <glm/gtc/type_ptr.hpp>

namespace cyclonite {
class Root;
}

namespace cyclonite::resources {
class ResourceManager;
}

namespace cyclonite::systems {
using namespace easy_mp;

class MeshSystem : public enttx::BaseSystem<MeshSystem>
{
public:
    using transfer_commands_t = vulkan::CommandBufferSet<vulkan::CommandPool, std::array<VkCommandBuffer, 3>>;

    using tag_t = easy_mp::type_list<components::Mesh>;

    MeshSystem() = default;

    MeshSystem(MeshSystem const&) = delete;

    MeshSystem(MeshSystem&&) = default;

    ~MeshSystem() = default;

    auto operator=(MeshSystem const&) -> MeshSystem& = delete;

    auto operator=(MeshSystem &&) -> MeshSystem& = default;

    auto createGeometry(uint32_t vertexCount, uint32_t indexCount) -> uint64_t;

    template<typename EntityManager, typename Geometries>
    auto createMesh(EntityManager& entityManager, enttx::Entity entity, Geometries&& geometries)
      -> std::enable_if_t<is_contiguous_v<Geometries> || std::is_same_v<uint64_t, std::decay_t<Geometries>>,
                          components::Mesh&>;

    // TODO:: delete Mesh

    void init(Root& root,
              size_t swapChainLength,
              size_t initialCommandCapacity,
              size_t initialInstanceCapacity,
              size_t initialIndexCapacity,
              size_t initialVertexCapacity);

    template<typename SystemManager, typename EntityManager, size_t STAGE, typename... Args>
    void update(SystemManager& systemManager, EntityManager& entityManager, Args&&... args);

    void requestVertexDeviceBufferUpdate();

private:
    void _addSubMesh(components::SubMesh& subMesh, uint64_t geometryId);

    void _reAllocCommandBuffer(size_t size);

    auto _getDumpCommandIndex() -> size_t;

private:
    vulkan::Device* devicePtr_;
    resources::ResourceManager* resourceManager_;
    VkQueue vkTransferQueue_;
    VkQueue vkGraphicQueue_;

    std::vector<VkDrawIndexedIndirectCommand> commands_;
    resources::Resource::Id commandBuffer_;
    std::shared_ptr<vulkan::Buffer> gpuCommandBuffer_;
    std::vector<size_t> commandDump_;
    uint32_t commandCount_;

    resources::Resource::Id instancedDataBuffer_;
    std::shared_ptr<vulkan::Buffer> gpuInstancedDataBuffer_;

    resources::Resource::Id indexBuffer_;
    std::shared_ptr<vulkan::Buffer> gpuIndexBuffer_;

    resources::Resource::Id vertexBuffer_;
    std::shared_ptr<vulkan::Buffer> gpuVertexBuffer_;

    std::vector<vulkan::Handle<VkSemaphore>> transferSemaphores_;
    std::unique_ptr<transfer_commands_t> transferCommands_;
    bool verticesUpdateRequired_;
};

template<typename EntityManager, typename Geometries>
auto MeshSystem::createMesh(EntityManager& entityManager, enttx::Entity entity, Geometries&& geometries)
  -> std::enable_if_t<is_contiguous_v<Geometries> || std::is_same_v<uint64_t, std::decay_t<Geometries>>,
                      components::Mesh&>
{
    auto subMeshCount = uint16_t{ 0 };
    auto geometryIdentifiers = std::add_pointer_t<uint64_t>{ nullptr };
    if constexpr (is_contiguous_v<Geometries>) {
        assert(std::size(geometries) < std::numeric_limits<uint16_t>::max());
        subMeshCount = static_cast<uint16_t>(std::size(geometries));
        geometryIdentifiers = std::data(geometries);
    } else {
        subMeshCount = uint16_t{ 1 };
        geometryIdentifiers = &geometries;
    }

    auto& mesh = entityManager.template assign<components::Mesh>(entity, subMeshCount);
    assert(subMeshCount == mesh.getSubMeshCount());

    for (auto i = uint16_t{ 0 }; i < subMeshCount; i++) {
        _addSubMesh(mesh.getSubMesh(i), *(geometryIdentifiers + i));
    }

    return mesh;
}

template<typename SystemManager, typename EntityManager, size_t STAGE, typename... Args>
void MeshSystem::update(SystemManager& systemManager, EntityManager& entityManager, Args&&... args)
{
    if constexpr (STAGE == value_cast(UpdateStage::EARLY_UPDATE)) {
        {
            auto view = entityManager.template getView<components::Mesh>();

            for (auto&& [entity, mesh] : view) {
                (void)entity;

                for (auto subMeshIndex = uint16_t{ 0 }, subMeshCount = mesh.getSubMeshCount();
                     subMeshIndex < subMeshCount;
                     subMeshIndex++) {
                    auto const& subMesh = mesh.getSubMesh(subMeshIndex);
                    commands_[subMesh.commandIndex].instanceCount++;
                }
            }
        }

        {
            auto& commandBuffer = resourceManager_->get(commandBuffer_).template as<resources::Staging>();
            auto* commands = reinterpret_cast<VkDrawIndexedIndirectCommand*>(commandBuffer.ptr());

            auto commandCount = size_t{ 0 };

            for (auto i = size_t{ 0 }, count = commands_.size(); i < count; i++) {
                if (commands_[i].instanceCount > 0) {
                    auto firstInstance = commandCount > 0 ? (*(commands + (commandCount - 1))).firstInstance +
                                                              (*(commands + (commandCount - 1))).instanceCount
                                                          : uint32_t{ 0 };
                    auto&& command = *(commands + commandCount++);

                    command.indexCount = commands_[i].indexCount;
                    command.instanceCount = commands_[i].instanceCount;
                    command.firstIndex = commands_[i].firstIndex;
                    command.vertexOffset = commands_[i].vertexOffset;
                    command.firstInstance = firstInstance;

                    commands_[i].firstInstance = firstInstance;
                    commands_[i].instanceCount = 0;
                } else {
                    commands_[i].firstInstance = std::numeric_limits<uint32_t>::max();
                }
            }

            commandCount_ = commandCount;
        }

        {
            auto& instanceDataBuffer = resourceManager_->get(instancedDataBuffer_).template as<resources::Staging>();
            auto* instanceData = reinterpret_cast<instanced_data_t*>(instanceDataBuffer.ptr());

            auto view = entityManager.template getView<components::Transform, components::Mesh>();

            for (auto&& [entity, transform, mesh] : view) {
                auto&& matrix = transform.worldMatrix;
                auto subMeshCount = mesh.getSubMeshCount();

                for (auto subMeshIndex = uint16_t{ 0 }; subMeshIndex < subMeshCount; subMeshIndex++) {
                    auto&& subMesh = mesh.getSubMesh(subMeshIndex);
                    auto&& command = commands_[subMesh.commandIndex];

                    if (command.firstInstance == std::numeric_limits<uint32_t>::max())
                        continue;

                    auto instance = instanceData + command.firstInstance + command.instanceCount++;

                    instance->transform1.x = matrix[0].x;
                    instance->transform1.y = matrix[1].x;
                    instance->transform1.z = matrix[2].x;
                    instance->transform1.w = matrix[3].x;

                    instance->transform2.x = matrix[0].y;
                    instance->transform2.y = matrix[1].y;
                    instance->transform2.z = matrix[2].y;
                    instance->transform2.w = matrix[3].y;

                    instance->transform3.x = matrix[0].z;
                    instance->transform3.y = matrix[1].z;
                    instance->transform3.z = matrix[2].z;
                    instance->transform3.w = matrix[3].z;
                }
            }
        }
    }

    if constexpr (STAGE == value_cast(UpdateStage::LATE_UPDATE)) {
        for (auto&& command : commands_) {
            command.instanceCount = 0;
        }
    }

    if constexpr (STAGE == value_cast(UpdateStage::TRANSFER_STAGE)) {
        auto&& [node, signalCount, frameNumber, dt] = std::forward_as_tuple(std::forward<Args>(args)...);

        auto& frame = node.getCurrentFrame();
        auto idx = node.frameBufferIndex();

        auto const& signal = std::as_const(transferSemaphores_[idx]);
        auto commandBufferCount = verticesUpdateRequired_ ? uint32_t{ 3 } : uint32_t{ 2 };
        auto baseSemaphore = node.waitSemaphores();
        auto baseDstStageMask = node.waitStages();

        *(baseSemaphore + signalCount) = static_cast<VkSemaphore>(signal);
        *(baseDstStageMask + signalCount) = VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;
        signalCount++;

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = commandBufferCount;
        submitInfo.pCommandBuffers = transferCommands_->pCommandBuffers();
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &signal;

        if (auto result = vkQueueSubmit(vkTransferQueue_, 1, &submitInfo, VK_NULL_HANDLE); result != VK_SUCCESS) {
            throw std::runtime_error("mesh system data could not be transferred");
        }

        frame.setIndexBuffer(devicePtr_->graphicsQueue(), gpuIndexBuffer_);
        frame.setVertexBuffer(devicePtr_->graphicsQueue(), gpuVertexBuffer_);
        frame.setInstanceBuffer(devicePtr_->graphicsQueue(), gpuInstancedDataBuffer_);
        frame.setCommandBuffer(devicePtr_->graphicsQueue(), gpuCommandBuffer_, commandCount_);
    }

    (void)systemManager;

    (void)entityManager;

    ((void)args, ...);
}
}

#endif // CYCLONITE_MESHSYSTEM_H
