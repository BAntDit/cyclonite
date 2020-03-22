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

    void init(vulkan::Device& device);

    template<typename SystemManager, typename EntityManager, size_t STAGE, typename... Args>
    void update(SystemManager& systemManager, EntityManager& entityManager, Args&&... args);

private:
    VkDevice vkDevice_;

    std::unique_ptr<vulkan::Staging> commandBuffer_;
    std::shared_ptr<vulkan::Buffer> gpuCommandBuffer_;
    std::unique_ptr<vulkan::Staging> transformBuffer_;
    std::shared_ptr<vulkan::Buffer> gpuTransformBuffer_;
    std::unique_ptr<vulkan::Staging> indicesBuffer_;
    std::shared_ptr<vulkan::Buffer> gpuIndicesBuffer_;
};

template<typename SystemManager, typename EntityManager, size_t STAGE, typename... Args>
void MeshSystem::update(SystemManager& systemManager, EntityManager& entityManager, Args&&... args)
{
    if constexpr (STAGE == easy_mp::value_cast(UpdateStage::EARLY_UPDATE)) {
        auto& commands = *reinterpret_cast<VkDrawIndexedIndirectCommand*>(commandBuffer_->ptr());

        commands.indexCount = 36;
        commands.instanceCount = 0;
        commands.firstIndex = 0;
        commands.vertexOffset = 0;
        commands.firstInstance = 0;
    }

    if constexpr (STAGE == easy_mp::value_cast(UpdateStage::LATE_UPDATE)) {
        auto&& [camera, dt] = std::forward_as_tuple(std::forward<Args>(args)...);
        (void)camera;
        (void)dt;

        auto* transforms3x4 = reinterpret_cast<real*>(transformBuffer_->ptr());
        auto& commands = *reinterpret_cast<VkDrawIndexedIndirectCommand*>(commandBuffer_->ptr());
        auto const& transformSystem = std::as_const(systemManager).template get<TransformSystem>();
        auto const& transforms = transformSystem.worldMatrices();

        auto view = entityManager.template getView<components::Transform, components::Mesh>();

        size_t dstIndex = 0;

        for (auto&& [entity, transform, mesh] : view) {
            (void)entity;
            (void)mesh;

            commands.instanceCount++;

            auto srcIndex = transform.globalIndex;

            std::copy_n(glm::value_ptr(glm::transpose(transforms[srcIndex])), 12, transforms3x4 + 12 * dstIndex++);
        }
    }
}
}

#endif // CYCLONITE_MESHSYSTEM_H
