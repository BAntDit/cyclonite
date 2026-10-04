//
// Created by anton on 10/3/26.
//

#ifndef CYCLONITE_SYSTEMS_FRAME_MANAGEMENT_SYSTEM_H
#define CYCLONITE_SYSTEMS_FRAME_MANAGEMENT_SYSTEM_H

#include "core/resourceSharedRef.h"
#include "stages.h"
#include <future>
#include <metrix/enum.h>

namespace cyclonite {
template<typename Config>
class Root;
}

namespace cyclonite::systems {
class FrameManagementSystem
{
public:
    FrameManagementSystem() = default;

    void init(core::ResourceSharedRef const& deviceRef);

    template<size_t ExecutionStage, typename Config>
    auto run(Root<Config>& root,
             std::shared_future<void>& prevStageFutures,
             core::ResourceSharedRef const& sceneRef) -> std::future<void>;

private:
    core::ResourceSharedRef deviceRef_;
};

template<size_t ExecutionStage, typename Config>
auto FrameManagementSystem::run(Root<Config>& root,
                                std::shared_future<void>& prevStageFutures,
                                core::ResourceSharedRef const& sceneRef) -> std::future<void>
{
    if constexpr (ExecutionStage == metrix::value_cast(SystemUpdateStageList::FRAME_START)) {
        // TODO::
    }
    // TODO::
}
}

#endif // CYCLONITE_SYSTEMS_FRAME_MANAGEMENT_SYSTEM_H