//
// Created by bantdit on 3/21/20.
//

#ifndef CYCLONITE_UNIFORMSYSTEM_H
#define CYCLONITE_UNIFORMSYSTEM_H

#include "../typedefs.h"
#include "renderSystem.h"
#include "resources/staging.h"
#include "updateStages.h"
#include <enttx/enttx.h>
#include <metrix/enum.h>

namespace cyclonite::systems {
class UniformSystem : public enttx::BaseSystem<UniformSystem>
{
public:
    UniformSystem() = default;

    UniformSystem(UniformSystem const&) = delete;

    UniformSystem(UniformSystem&&) = default;

    ~UniformSystem() = default;

    auto operator=(UniformSystem const&) -> UniformSystem& = delete;

    auto operator=(UniformSystem&&) -> UniformSystem& = default;

    void init(multithreading::TaskManager& taskManager,
              resources::ResourceManager& resourceManager,
              size_t swapChainLength);

    [[nodiscard]] auto uniforms() const -> resources::Staging const&;

    auto uniforms() -> resources::Staging&;

    template<typename SystemManager, typename EntityManager, size_t STAGE, typename... Args>
    void update(SystemManager& systemManager, EntityManager& entityManager, Args&&... args);

    // tmp
    void setViewMatrix(mat4& viewMatrix);

    void setProjectionMatrix(mat4& projectionMatrix);

    void setViewProjectionMatrix(mat4& viewProjMatrix);

private:
    void _init(resources::ResourceManager& resourceManager, size_t swapChainLength);
    VkQueue vkTransferQueue_;
};

template<typename SystemManager, typename EntityManager, size_t STAGE, typename... Args>
void UniformSystem::update(SystemManager& systemManager, EntityManager& entityManager, Args&&... args)
{
    (void)entityManager;
    (void)systemManager;

    ((void)args, ...);
}
}

#endif // CYCLONITE_UNIFORMSYSTEM_H
