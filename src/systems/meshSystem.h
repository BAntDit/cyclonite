//
// Created by bantdit on 2/6/20.
//

#ifndef CYCLONITE_MESHSYSTEM_H
#define CYCLONITE_MESHSYSTEM_H

#include "../components/mesh.h"
#include "../components/transform.h"
#include "../geometry.h"
#include "transformSystem.h"
#include "vulkan/buffer.h"
#include "vulkan/commandBufferSet.h"
#include "vulkan/commandPool.h"
#include "vulkan/staging.h"
#include <easy-mp/enum.h>
#include <enttx/enttx.h>
#include <glm/gtc/type_ptr.hpp>
#include <set>

namespace cyclonite::systems {
using namespace easy_mp;

class MeshSystem : public enttx::BaseSystem<MeshSystem>
{
public:
    using transfer_commands_t = vulkan::CommandBufferSet<vulkan::CommandPool, std::array<VkCommandBuffer, 1>>;

    using tag_t = easy_mp::type_list<components::Mesh>;

    MeshSystem() = default;

    MeshSystem(MeshSystem const&) = delete;

    MeshSystem(MeshSystem&&) = default;

    ~MeshSystem() = default;

    auto operator=(MeshSystem const&) -> MeshSystem& = delete;

    auto operator=(MeshSystem &&) -> MeshSystem& = default;

    auto createGeometry(uint32_t vertexCount, uint32_t indexCount) -> std::shared_ptr<Geometry>;

    template<typename EntityManager, typename SubMeshData>
    auto createMesh(EntityManager& entityManager, enttx::Entity entity, SubMeshData&& subMeshData)
      -> std::enable_if_t<is_contiguous_v<SubMeshData>, components::Mesh&>;

    template<typename SubMeshData>
    auto addSubMeshes(components::Mesh& mesh, SubMeshData&& subMeshData)
      -> std::enable_if_t<is_contiguous_v<SubMeshData>>;

    // TODO:: update submesh vertices method

    void markToDeleteMesh();

    void markToDeleteSubMesh();

    void init(vulkan::Device& device,
              size_t initialCommandCapacity,
              size_t initialInstanceCapacity,
              size_t initialIndexCapacity,
              size_t initialVertexCapacity);

    template<typename SystemManager, typename EntityManager, size_t STAGE, typename... Args>
    void update(SystemManager& systemManager, EntityManager& entityManager, Args&&... args);

private:
    void _addSubMesh(components::Mesh& mesh, uint32_t indexCount, uint32_t vertexCount);

    void _reAllocCommandBuffer(size_t size);

private:
    vulkan::Device* devicePtr_;

    std::unique_ptr<vulkan::Staging> commandBuffer_;
    std::shared_ptr<vulkan::Buffer> gpuCommandBuffer_;
    std::vector<size_t> commandDump_;
    size_t commandCount_ = 0;

    std::unique_ptr<vulkan::Staging> instancedDataBuffer_;
    std::shared_ptr<vulkan::Buffer> gpuInstancedDataBuffer_;

    std::unique_ptr<vulkan::Staging> indexBuffer_;
    std::shared_ptr<vulkan::Buffer> gpuIndexBuffer_;

    std::unique_ptr<vulkan::Staging> vertexBuffer_;
    std::shared_ptr<vulkan::Buffer> gpuVertexBuffer_;

    std::set<std::shared_ptr<Geometry>> geometry_;

    bool isVertexElementBuffersInActualState_;
};

template<typename EntityManager, typename SubMeshData>
auto MeshSystem::createMesh(EntityManager& entityManager, enttx::Entity entity, SubMeshData&& subMeshData)
  -> std::enable_if_t<is_contiguous_v<SubMeshData>, components::Mesh&>
{
    auto& mesh = entityManager.template assign<components::Mesh>(entity);

    addSubMeshes(mesh, std::forward<SubMeshData>(subMeshData));

    return mesh;
}

template<typename SubMeshData>
auto MeshSystem::addSubMeshes(components::Mesh& mesh, SubMeshData&& subMeshData)
  -> std::enable_if_t<is_contiguous_v<SubMeshData>>
{
    if (!subMeshData.empty()) {
        if (commandCount_ * sizeof(VkDrawIndexedIndirectCommand) - commandBuffer_->size() <
            subMeshData.size() * sizeof(VkDrawIndexedIndirectCommand)) {
            _reAllocCommandBuffer((commandCount_ + subMeshData.size()) * sizeof(VkDrawIndexedIndirectCommand));
        }
    }

    mesh.subMeshes.reserve(mesh.subMeshes.size() + subMeshData.size());

    for (auto&& value : subMeshData) {
        auto&& [ic, vc] = value;
        _addSubMesh(mesh, ic, vc);
    }
}

template<typename SystemManager, typename EntityManager, size_t STAGE, typename... Args>
void MeshSystem::update(SystemManager& systemManager, EntityManager& entityManager, Args&&... args)
{
    (void)systemManager;

    (void)entityManager;

    ((void)args, ...);
}
}

#endif // CYCLONITE_MESHSYSTEM_H
