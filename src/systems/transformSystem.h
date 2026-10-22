
#ifndef CYCLONITE_SYSTEMS_TRANSFORM_SYSTEM_H
#define CYCLONITE_SYSTEMS_TRANSFORM_SYSTEM_H

#include "core/resourceSharedRef.h"
#include <future>
#include <metrix/enum.h>
#include "stages.h"

namespace cyclonite {
template<typename Config>
class Root;
}

namespace cyclonite::systems {
class TransformSystem
{
public:
    TransformSystem() = default;

    template<size_t ExecutionStage, typename Config>
    auto run(Root<Config>& root, std::shared_future<void>& prevStageFutures, core::ResourceSharedRef const& sceneRef)
      -> std::future<void>;

private:
    template<typename Config>
    void updateTransforms(Root<Config>& root, core::ResourceSharedRef const& sceneRef);
};

template<size_t ExecutionStage, typename Config>
auto TransformSystem::run(Root<Config>& root,
                          std::shared_future<void>& prevStageFutures,
                          core::ResourceSharedRef const& sceneRef) -> std::future<void>
{
    if constexpr (ExecutionStage == metrix::value_cast(SystemUpdateStageList::EARLY_UPDATE)) {
        
    } else {
        auto promise = std::promise<void>();
        auto future = promise.get_future();

        promise.set_value();

        return future;
    }
}
}

#ifdef CYCLONITE_SYSTEMS_TRANSFORM_SYSTEM_IMPLEMENTATION
#include "enttx/entityManager.h"
#include "root.h"

namespace cyclonite::systems {
template<typename Config>
void TransformSystem::updateTransforms(Root<Config>& root, core::ResourceSharedRef const& sceneRef)
{
    auto& ecs = root.getSceneEntities(sceneRef);

    auto view = entityManager.template getView<components::Transform>();

    for (auto&& [entity, transform] : view) {
    
    }
}
}

#endif

#endif // CYCLONITE_SYSTEMS_TRANSFORM_SYSTEM_H
