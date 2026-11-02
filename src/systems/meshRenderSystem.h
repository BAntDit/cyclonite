//
// Created by anton on 11/2/26.
//

#ifndef CYCLONITE_SYSTEMS_MESH_RENDER_SYSTEM_H
#define CYCLONITE_SYSTEMS_MESH_RENDER_SYSTEM_H

#include "root.h"

namespace cyclonite::systems {
class MeshRenderSystem
{
public:


    template<size_t ExecutionStage, typename Config>
    auto run(Root<Config>& root,
             std::shared_future<void>& prevStageFutures,
             core::ResourceSharedRef const& sceneRef) -> std::future<void>;

private:
    // TODO:: place command buffer stagings with respect to pipeline id / DIP type / scene
};
}

#endif // CYCLONITE_SYSTEMS_MESH_RENDER_SYSTEM_H
