//
// Created by bantdit on 2/23/20.
//

#ifndef CYCLONITE_CAMERASYSTEM_H
#define CYCLONITE_CAMERASYSTEM_H

#include "../components/camera.h"
#include "transformSystem.h"
#include "uniformSystem.h"
#include "updateStages.h"
#include "vulkan/commandBufferSet.h"
#include "vulkan/commandPool.h"
#include "vulkan/staging.h"
#include <components/transform.h>
#include <easy-mp/enum.h>
#include <enttx/enttx.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace cyclonite::systems {
class CameraSystem : public enttx::BaseSystem<CameraSystem>
{
public:
    using transfer_commands_t = vulkan::CommandBufferSet<vulkan::CommandPool, std::array<VkCommandBuffer, 1>>;

    using tag_t = easy_mp::type_list<components::Camera>;

    CameraSystem() = default;

    CameraSystem(CameraSystem const&) = delete;

    CameraSystem(CameraSystem&&) = default;

    ~CameraSystem() = default;

    auto operator=(CameraSystem const&) -> CameraSystem& = delete;

    auto operator=(CameraSystem &&) -> CameraSystem& = default;

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
};

template<typename SystemManager, typename EntityManager, size_t STAGE, typename... Args>
void CameraSystem::update(SystemManager& systemManager, EntityManager& entityManager, Args&&... args)
{
    using namespace easy_mp;

    if constexpr (STAGE == value_cast(UpdateStage::LATE_UPDATE)) {
        std::cout << "camera system: late update: start" << std::endl;

        auto&& [cameraEntity, dt] = std::forward_as_tuple(std::forward<Args>(args)...);
        (void)dt;

        auto& uniformSystem = systemManager.template get<UniformSystem>();
        auto const& transformSystem = std::as_const(systemManager).template get<TransformSystem>();
        auto const& transforms = transformSystem.worldMatrices();

        auto [transform, camera] =
          std::as_const(entityManager).template getComponents<components::Transform, components::Camera>(cameraEntity);

        auto viewMatrix = glm::inverse(transforms[transform->globalIndex]);

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

        auto viewProjectionMatrix = glm::transpose(projectionMatrix) * viewMatrix;

        uniformSystem.setViewMatrix(viewMatrix);

        uniformSystem.setProjectionMatrix(projectionMatrix);

        uniformSystem.setViewProjectionMatrix(viewProjectionMatrix);

        std::cout << "camera system: late update: end" << std::endl;
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
