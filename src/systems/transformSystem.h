
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

private:
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

    // TODO::
}
}
#endif // CYCLONITE_SYSTEMS_TRANSFORM_SYSTEM_H