//
// Created by bantdit on 2/23/20.
//

#ifndef CYCLONITE_CAMERASYSTEM_H
#define CYCLONITE_CAMERASYSTEM_H

#include "../components/camera.h"
#include "transformSystem.h"
#include "updateStages.h"
#include "vulkan/commandBufferSet.h"
#include "vulkan/commandPool.h"
#include "vulkan/staging.h"
#include <components/transform.h>
#include <easy-mp/enum.h>
#include <enttx/enttx.h>
#include <glm/gtc/type_ptr.hpp>

namespace cyclonite::systems {
class CameraSystem : public enttx::BaseSystem<CameraSystem>
{
public:
    using transfer_commands_t = vulkan::CommandBufferSet<vulkan::CommandPool, std::array<VkCommandBuffer, 1>>;

    using tag_t = easy_mp::type_list<components::Camera>;

    CameraSystem() = default;

    ~CameraSystem() = default;

    void init(vulkan::Device& device);

    template<typename SystemManager, typename EntityManager, size_t STAGE, typename... Args>
    void update(SystemManager& systemManager, EntityManager& entityManager, Args&&... args);

private:
    std::unique_ptr<vulkan::Staging> uniforms_;
    std::unique_ptr<vulkan::Buffer> gpuUniforms_;
    std::shared_ptr<transfer_commands_t> persistentTransfer_;
    vulkan::Handle<VkSemaphore> transferSemaphore_;
};

template<typename SystemManager, typename EntityManager, size_t STAGE, typename... Args>
void CameraSystem::update(SystemManager& systemManager, EntityManager& entityManager, Args&&... args)
{
    if constexpr (STAGE == easy_mp::value_cast(UpdateStage::EARLY_UPDATE)) {
        auto&& [frame, cameraEntity, dt] = std::forward_as_tuple(std::forward<Args>(args)...);
        (void)frame;
        (void)dt;

        auto* uniforms = reinterpret_cast<real*>(uniforms_->ptr());
        auto const& transformSystem = std::as_const(systemManager).template get<TransformSystem>();
        auto const& transforms = transformSystem.worldMatrices();

        auto [transform, camera] =
          std::as_const(entityManager).template getComponents<components::Transform, components::Camera>(cameraEntity);

        auto viewMatrix = glm::inverse(transforms[transform.globalIndex]);

        auto projectionMatrix = std::visit(
          [](auto&& projection) -> mat4 {
              if constexpr (std::is_same_v<std::decay_t<decltype(projection)>,
                                           components::Camera::PerspectiveProjection>) {
                  auto& [aspect, yFov, zNear, zFar] = projection;
                  return glm::perspective(aspect, yFov, zNear, zFar);
              }

              if constexpr (std::is_same_v<std::decay_t<decltype(projection)>,
                                           components::Camera::OrthographicProjection>) {
                  auto& [xMag, yMag, zNear, zFar] = projection;
                  return glm::ortho(0.0, xMag, 0.0, yMag, zNear, zFar);
              }
          },
          camera.projection);

        auto viewProjectionMatrix = projectionMatrix * viewMatrix;

        std::copy_n(glm::value_ptr(viewMatrix), 16, uniforms);
        std::copy_n(glm::value_ptr(projectionMatrix), 16, uniforms + 16);
        std::copy_n(glm::value_ptr(viewProjectionMatrix), 16, uniforms + 32);
    }
}
}

#endif // CYCLONITE_CAMERASYSTEM_H
