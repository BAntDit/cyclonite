//
// Created by bantdit on 2/6/20.
//

#ifndef CYCLONITE_MESHSYSTEM_H
#define CYCLONITE_MESHSYSTEM_H

#include "../components/mesh.h"
#include "updateStages.h"
#include <easy-mp/enum.h>
#include <enttx/enttx.h>
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
    std::unique_ptr<vulkan::Buffer> gpuCommandBuffer_;
    std::unique_ptr<vulkan::Buffer> gpuIndicesBuffer_;
    std::unique_ptr<vulkan::CommandBufferSet<std::array<VkCommandBuffer, 1>>> transferCommands_;
};

template<typename SystemManager, typename EntityManager, size_t STAGE>
void MeshSystem::update(SystemManager& systemManager, EntityManager& entityManager)
{}
}

#endif // CYCLONITE_MESHSYSTEM_H
