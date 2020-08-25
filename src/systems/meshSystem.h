//
// Created by bantdit on 2/6/20.
//

#ifndef CYCLONITE_MESHSYSTEM_H
#define CYCLONITE_MESHSYSTEM_H

#include "../components/mesh.h"
#include "../components/transform.h"
#include "transformSystem.h"
#include "vulkan/buffer.h"
#include "vulkan/commandBufferSet.h"
#include "vulkan/commandPool.h"
#include "vulkan/staging.h"
#include <easy-mp/enum.h>
#include <enttx/enttx.h>
#include <glm/gtc/type_ptr.hpp>

namespace cyclonite::systems {
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

    template<typename EntityManager>
    auto createMesh(EntityManager& entityManager, enttx::Entity entity) -> components::Mesh const&;

    void markToDeleteMesh();

    void addSubMesh();

    void markToDeleteSubMesh();

    void init(vulkan::Device& device,
              size_t initialCommandCount,
              size_t initialInstanceCount,
              size_t initialIndexCount,
              size_t initialVertexCount);

    template<typename SystemManager, typename EntityManager, size_t STAGE, typename... Args>
    void update(SystemManager& systemManager, EntityManager& entityManager, Args&&... args);

private:
    VkDevice vkDevice_;

    std::unique_ptr<vulkan::Staging> commandBuffer_;
    std::shared_ptr<vulkan::Buffer> gpuCommandBuffer_;
    std::unique_ptr<vulkan::Staging> instancedDataBuffer_;
    std::shared_ptr<vulkan::Buffer> gpuInstancedDataBuffer_;
    std::unique_ptr<vulkan::Staging> indexBuffer_;
    std::shared_ptr<vulkan::Buffer> gpuIndexBuffer_;
    std::unique_ptr<vulkan::Staging> vertexBuffer_;
    std::shared_ptr<vulkan::Buffer> gpuVertexBuffer_;
    std::vector<components::Mesh::SubMesh> subMeshes_;
};

template<typename EntityManager>
auto MeshSystem::createMesh(EntityManager& entityManager, enttx::Entity entity) -> components::Mesh const& {
    auto& mesh = entityManager.template assign<components::Mesh>();

    mesh.firstSubMeshIndex = subMeshes_.size();
    mesh.subMeshCount = 0;

    return mesh;
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
