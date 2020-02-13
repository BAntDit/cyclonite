//
// Created by bantdit on 2/6/20.
//

#ifndef CYCLONITE_MESHSYSTEM_H
#define CYCLONITE_MESHSYSTEM_H

#include "../components/mesh.h"
#include "../components/transform.h"
#include "transformSystem.h"
#include <easy-mp/enum.h>
#include <enttx/enttx.h>
#include <glm/gtc/type_ptr.hpp>
#include <vulkan/buffer.h>
#include <vulkan/commandBufferSet.h>
#include <vulkan/staging.h>

namespace cyclonite::systems {
class MeshSystem : public enttx::BaseSystem<MeshSystem>
{
public:
    using tag_t = easy_mp::type_list<components::Mesh>;

    MeshSystem() = default;

    ~MeshSystem() = default;

    void init(vulkan::Device& device);

    template<typename SystemManager, typename EntityManager, size_t STAGE>
    void update(SystemManager& systemManager, EntityManager& entityManager);

private:
    std::unique_ptr<vulkan::Staging> commandBuffer_;
    std::unique_ptr<vulkan::Staging> indicesBuffer_;
    std::unique_ptr<vulkan::Staging> transformBuffer_;
    std::unique_ptr<vulkan::Buffer> gpuCommandBuffer_;
    std::unique_ptr<vulkan::Buffer> gpuIndicesBuffer_;
    std::unique_ptr<vulkan::CommandBufferSet<std::array<VkCommandBuffer, 1>>> transferCommands_;
};

template<typename SystemManager, typename EntityManager, size_t STAGE>
void MeshSystem::update(SystemManager& systemManager, EntityManager& entityManager)
{
    if constexpr (STAGE == easy_mp::value_cast(UpdateStage::LATE_UPDATE)) {
        auto* transforms3x4 = reinterpret_cast<real*>(transformBuffer_->ptr());
        auto& commands = *reinterpret_cast<VkDrawIndexedIndirectCommand*>(commandBuffer_->ptr());
        auto const& transformSystem = std::as_const(systemManager).template get<TransformSystem>();
        auto const& transforms = transformSystem.worldMatrices();

        commands.indexCount = 36;
        commands.instanceCount = 0;
        commands.firstIndex = 0;
        commands.vertexOffset = 0;
        commands.firstInstance = 0;

        auto view = entityManager.template getView<components::Transform, components::Mesh>();

        size_t dstIndex = 0;

        for (auto& [entity, transform, mesh] : view) {
            (void)entity;
            (void)mesh;

            commands.instanceCount++;

            auto srcIndex = transform.globalIndex;

            std::copy_n(
              glm::value_ptr(glm::transpose(transforms[srcIndex])), 12, transforms3x4 + 12 * dstIndex++);
        }
    }
}
}

#endif // CYCLONITE_MESHSYSTEM_H
