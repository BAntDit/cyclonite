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

    template<typename EntityManager, typename Geometries>
    auto createMesh(EntityManager& entityManager, enttx::Entity entity, Geometries&& geometries)
      -> std::enable_if_t<is_contiguous_v<Geometries> ||
                            std::is_same_v<std::shared_ptr<Geometry>, std::decay_t<Geometries>>,
                          components::Mesh&>;

    template<typename Geometries>
    auto addSubMeshes(components::Mesh& mesh, Geometries&& geometries)
      -> std::enable_if_t<is_contiguous_v<Geometries> ||
                          std::is_same_v<std::shared_ptr<Geometry>, std::decay_t<Geometries>>>;

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
    void _addSubMesh(components::Mesh& mesh, std::shared_ptr<Geometry> const& geometry);

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

    std::set<std::shared_ptr<Geometry>> geometries_;

    bool isVertexElementBuffersInActualState_;
};

template<typename EntityManager, typename Geometries>
auto MeshSystem::createMesh(EntityManager& entityManager, enttx::Entity entity, Geometries&& geometries)
  -> std::enable_if_t<is_contiguous_v<Geometries> ||
                        std::is_same_v<std::shared_ptr<Geometry>, std::decay_t<Geometries>>,
                      components::Mesh&>
{
    auto& mesh = entityManager.template assign<components::Mesh>(entity);

    addSubMeshes(mesh, std::forward<Geometries>(geometries));

    return mesh;
}

template<typename Geometries>
auto MeshSystem::addSubMeshes(components::Mesh& mesh, Geometries&& geometries)
  -> std::enable_if_t<is_contiguous_v<Geometries> ||
                      std::is_same_v<std::shared_ptr<Geometry>, std::decay_t<Geometries>>>
{
    if constexpr (is_contiguous_v<Geometries>) {
        mesh.subMeshes.reserve(geometries.size());
    }

    for (auto&& geometry : geometries) {
        _addSubMesh(mesh, std::forward<decltype(geometry)>(geometry));
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
