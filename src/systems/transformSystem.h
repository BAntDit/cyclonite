
#ifndef CYCLONITE_SYSTEMS_TRANSFORM_SYSTEM_H
#define CYCLONITE_SYSTEMS_TRANSFORM_SYSTEM_H

#include "components/transformStorage.h"
#include "core/resourceSharedRef.h"
#include "enttx/entityManager.h"
#include "root.h"
#include "scene.h"
#include "stages.h"
#include <future>
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

    static void _updateTransform(components::Transform const& parent, components::Transform& transform);

private:
    static constexpr size_t parallel_update_threshold_v = 64;

    template<typename EntityManagerConfig>
    auto updateSceneTreeLevel(enttx::EntityManager<EntityManagerConfig>& entityManager,
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

        return updateSceneTreeLevel(root.getSceneEntities(sceneRef), 0, 0);
    } else if constexpr (ExecutionStage == metrix::value_cast(SystemUpdateStageList::LATE_UPDATE)) {
        if (prevStageFutures.valid()) {
            prevStageFutures.get();
        }
        auto& ecs = root.getSceneEntities(sceneRef);
        auto view = ecs.template getView<components::Transform>();

        for (auto&& [entity, transform] : view) {
            transform.state() = components::Transform::State::UPDATE_NOTHING;
        }
    }

    auto promise = std::promise<void>();
    auto future = promise.get_future();

    promise.set_value();

    return future;
}

template<typename EntityManagerConfig>
auto TransformSystem::updateSceneTreeLevel(enttx::EntityManager<EntityManagerConfig>& entityManager,
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
        auto updateFutures = std::vector<std::future<void>>{};
        auto taskCount = levelTransformCount / parallel_update_threshold_v +
                         ((levelTransformCount % parallel_update_threshold_v) != 0);

        updateFutures.reserve(taskCount);

        auto start = levelStart;

        do {
            auto componentCount = taskCount > parallel_update_threshold_v ? parallel_update_threshold_v : taskCount;

            updateFutures.emplace_back() = multithreading::Executor::threadExecutor().submitTask(
              [&storage, &entityManager, start, componentCount]() -> void {
                  auto beginIt = std::next(storage.begin(), static_cast<long>(start));
                  auto endIt = std::next(storage.begin(), static_cast<long>(start + componentCount));

                  for (auto it = beginIt; it != endIt; ++it) {
                      auto& transform = *it;

                      auto const& parentTransform =
                        (transform.parent() != enttx::Entity{})
                          ? entityManager.template getComponent<components::Transform>(transform.parent())
                          : components::Transform{};

                      if (transform.state() == components::Transform::State::UPDATE_NOTHING &&
                          parentTransform.state() == components::Transform::State::UPDATE_NOTHING) {
                          continue;
                      }

                      TransformSystem::_updateTransform(parentTransform, transform);
                  }
              });

            start += componentCount;

            assert(componentCount <= taskCount);
            taskCount -= componentCount;
        } while (taskCount > 0);

        auto updateFuture = multithreading::when_all(updateFutures);
        if (isLastLevel) {
            return updateFuture;
        }
        updateFuture.get();

        return updateSceneTreeLevel(entityManager, levelStart + levelTransformCount, nextLevel);
    }

    auto beginIt = std::next(storage.begin(), static_cast<long>(levelStart));
    auto endIt = std::next(storage.begin(), static_cast<long>(levelStart + levelTransformCount));
    for (auto it = beginIt; it != endIt; ++it) {
        auto& transform = *it;

        auto const& parentTransform = (transform.parent() != enttx::Entity{})
                                        ? entityManager.template getComponent<components::Transform>(transform.parent())
                                        : components::Transform{};

        if (transform.state() == components::Transform::State::UPDATE_NOTHING &&
            parentTransform.state() == components::Transform::State::UPDATE_NOTHING) {
            continue;
        }

        TransformSystem::_updateTransform(parentTransform, transform);
    }

    if (isLastLevel) {
        return updateSceneTreeLevel(entityManager, levelStart + levelTransformCount, nextLevel);
    }

    auto promise = std::promise<void>();
    auto future = promise.get_future();

    promise.set_value();

    return future;
}
}
#endif // CYCLONITE_SYSTEMS_TRANSFORM_SYSTEM_H