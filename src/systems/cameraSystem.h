//
// Created by bantdit on 2/23/20.
//

#ifndef CYCLONITE_CAMERASYSTEM_H
#define CYCLONITE_CAMERASYSTEM_H

#include "../components/camera.h"
#include "resources/staging.h"
#include "transformSystem.h"
#include "uniformSystem.h"
#include "updateStages.h"
#include "vulkan/commandBufferSet.h"
#include "vulkan/commandPool.h"
#include <components/transform.h>
#include <enttx/enttx.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <metrix/enum.h>
#include <metrix/type_list.h>

namespace cyclonite::systems {
class CameraSystem : public enttx::BaseSystem<CameraSystem>
{
public:
    using transfer_commands_t = vulkan::CommandBufferSet<vulkan::CommandPool, std::array<VkCommandBuffer, 1>>;

    using tag_t = metrix::type_list<components::Camera>;

    CameraSystem() = default;

    CameraSystem(CameraSystem const&) = delete;

    CameraSystem(CameraSystem&&) = default;

    ~CameraSystem() = default;

    auto operator=(CameraSystem const&) -> CameraSystem& = delete;

    auto operator=(CameraSystem&&) -> CameraSystem& = default;

    void init();

    template<typename SystemManager, typename EntityManager, size_t STAGE, typename... Args>
    void update(SystemManager& systemManager, EntityManager& entityManager, Args&&... args);

    template<typename EntityManager>
    auto createCamera(EntityManager& entityManager,
                      enttx::Entity entity,
                      components::Camera::PerspectiveProjection const& perspective) -> components::Camera&;

    template<typename EntityManager>
    auto createCamera(EntityManager& entityManager,
                      enttx::Entity entity,
                      components::Camera::OrthographicProjection const& orthographic) -> components::Camera&;

    [[nodiscard]] auto renderCamera() const -> enttx::Entity { return renderCamera_; }
    auto renderCamera() -> enttx::Entity& { return renderCamera_; }

private:
    enttx::Entity renderCamera_;
};

template<typename SystemManager, typename EntityManager, size_t STAGE, typename... Args>
void CameraSystem::update(SystemManager& systemManager, EntityManager& entityManager, Args&&... args)
{
    ((void)args, ...);

    if constexpr (STAGE == metrix::value_cast(UpdateStage::LATE_UPDATE)) {
        auto [transform, camera] = std::as_const(entityManager)
                                     .template getComponents<components::Transform, components::Camera>(renderCamera());

        auto viewMatrix = glm::inverse(transform->worldMatrix);

        auto projectionMatrix = std::visit(
          [](auto&& projection) -> mat4 {
              if constexpr (std::is_same_v<std::decay_t<decltype(projection)>,
                                           components::Camera::PerspectiveProjection>) {
                  auto& [aspect, yFov, zNear, zFar] = projection;

                  real f = 1.0f / tan(0.5f * yFov);

                  return glm::transpose(mat4{ f / aspect,
                                              0.f,
                                              0.f,
                                              0.f,
                                              0.f,
                                              -f,
                                              0.f,
                                              0.f,
                                              0.f,
                                              0.f,
                                              zFar / (zNear - zFar),
                                              -1.f,
                                              0.f,
                                              0.f,
                                              (zNear * zFar) / (zNear - zFar),
                                              0.f });
              }

              if constexpr (std::is_same_v<std::decay_t<decltype(projection)>,
                                           components::Camera::OrthographicProjection>) {
                  auto& [xMag, yMag, zNear, zFar] = projection;
                  return glm::ortho(0.0f, xMag, 0.0f, yMag, zNear, zFar);
              }
          },
          camera->projection);

        auto viewProjectionMatrix = glm::transpose(projectionMatrix) * viewMatrix; // ?? transpose two times

        auto& uniformSystem = systemManager.template get<UniformSystem>();

        uniformSystem.setViewMatrix(viewMatrix);

        uniformSystem.setProjectionMatrix(projectionMatrix);

        uniformSystem.setViewProjectionMatrix(viewProjectionMatrix);
    }
}

template<typename EntityManager>
auto CameraSystem::createCamera(EntityManager& entityManager,
                                enttx::Entity entity,
                                components::Camera::PerspectiveProjection const& perspective) -> components::Camera&
{
    return entityManager.template assign<components::Camera>(entity, perspective);
}

template<typename EntityManager>
auto CameraSystem::createCamera(EntityManager& entityManager,
                                enttx::Entity entity,
                                components::Camera::OrthographicProjection const& orthographic) -> components::Camera&
{
    return entityManager.template assign<components::Camera>(entity, orthographic);
}
}

#endif // CYCLONITE_CAMERASYSTEM_H
