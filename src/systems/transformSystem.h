
#ifndef CYCLONITE_SYSTEMS_TRANSFORM_SYSTEM_H
#define CYCLONITE_SYSTEMS_TRANSFORM_SYSTEM_H

#include "components/transformStorage.h"
#include "core/resourceSharedRef.h"
#include "enttx/entityManager.h"
#include "root.h"
#include "scene.h"
#include "stages.h"
#include <future>
#include <glm/gtx/matrix_decompose.hpp>
#include <metrix/enum.h>

namespace cyclonite::systems {
class TransformSystem
{
public:
    TransformSystem() = default;

    template<size_t ExecutionStage, typename Config>
    auto run(Root<Config>& root,
             std::shared_future<void>& prevStageFutures,
             core::ResourceSharedRef const& sceneRef) -> std::future<void>;

private:
    static constexpr size_t parallel_update_threshold_v = 64;

    template<typename EntityManagerConfig>
    auto updateSceneTreeLevel(enttx::EntityManager<EntityManagerConfig>& entityManager,
                              std::vector<mat4>& worldMatrices,
                              size_t levelStart,
                              uint32_t level) -> std::future<void>;
};

template<size_t ExecutionStage, typename Config>
auto TransformSystem::run(Root<Config>& root,
                          std::shared_future<void>& prevStageFutures,
                          core::ResourceSharedRef const& sceneRef) -> std::future<void>
{
    if constexpr (ExecutionStage == metrix::value_cast(SystemUpdateStageList::EARLY_UPDATE)) {
        if (prevStageFutures.valid()) {
            prevStageFutures.get();
        }

        auto sceneCopyRef = sceneRef; // removes const
        auto& ecs = root.getSceneEntities(sceneRef);
        auto& scene = sceneCopyRef.as<Scene>();
        auto& worldMatrices = scene.nodeWorldMatrices();

        return updateSceneTreeLevel(ecs, worldMatrices, 0, 0);
    } else {
        auto promise = std::promise<void>();
        auto future = promise.get_future();

        promise.set_value();

        return future;
    }
}

template<typename EntityManagerConfig>
auto TransformSystem::updateSceneTreeLevel(enttx::EntityManager<EntityManagerConfig>& entityManager,
                                           std::vector<mat4>& worldMatrices,
                                           size_t levelStart,
                                           uint32_t level) -> std::future<void>
{
    auto isLastLevel = false;
    auto nextLevel = level + 1;
    auto levelTransformCount = size_t{ 0 };

    auto& storage = entityManager.template getStorage<components::TransformStorage>();

    if (level == 0) {
        assert(storage.size() > 0);

        levelTransformCount = 1;
        isLastLevel = (storage.size() < 2);
    } else {
        auto levelStartIt = std::next(storage.begin(), static_cast<long>(levelStart));
        auto nextLevelStartIt = std::upper_bound(
          levelStartIt, storage.end(), level, [](auto lhs, auto&& rhs) -> bool { return lhs < rhs.level(); });

        isLastLevel = (nextLevelStartIt != storage.end());
        levelTransformCount = std::distance(levelStartIt, nextLevelStartIt);
    }

    if (levelTransformCount >= parallel_update_threshold_v) {

    } else {
        auto beginIt = std::next(storage.begin(), static_cast<long>(levelStart));
        auto endIt = std::next(storage.begin(), static_cast<long>(levelStart + levelTransformCount));
        for (auto it = beginIt; it != endIt; ++it) {
            auto& transform = *it;
            auto parentWorldMatrix = mat4{ 1.f };

            auto const& parentTransform =
              (transform.parent() != enttx::Entity{})
                ? entityManager.template getComponent<components::Transform>(transform.parent())
                : components::Transform{};

            if (transform.state() == components::Transform::State::UPDATE_NOTHING &&
                parentTransform.state() == components::Transform::State::UPDATE_NOTHING) {
                continue;
            }

            if (transform.state() == components::Transform::State::UPDATE_LOCAL) {
                transform.setMatrix(glm::translate(transform.position()) * glm::mat4_cast(transform.orientation()) *
                                    glm::scale(transform.scale()));
                transform.state() = components::Transform::State::UPDATE_WORLD;
            }

            if (transform.state() == components::Transform::State::UPDATE_COMPONENTS) {
                [[maybe_unused]] auto skew = vec3{};
                [[maybe_unused]] auto perspective = vec4{};

                auto scale = vec3{};
                auto position = vec3{};
                auto rotation = quat{};

                [[maybe_unused]] bool success =
                  glm::decompose(transform.matrix(), scale, rotation, position, skew, perspective);
                assert(success);

                transform.setScale(scale);
                transform.setPosition(position);
                transform.setOrientation(rotation);

                transform.state() = components::Transform::State::UPDATE_WORLD;
            }

            if (transform.state() == components::Transform::State::UPDATE_WORLD || parentTransform.state() == components::Transform::State::UPDATE_CHILDREN) {
                // TODO::
                transform.state() = components::Transform::State::UPDATE_CHILDREN;
            }
        }

        if (isLastLevel) {

        } else {

        }
    }
}
}
#endif // CYCLONITE_SYSTEMS_TRANSFORM_SYSTEM_H