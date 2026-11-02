//
// Created by anton on 11/2/26.
//

#ifndef CYCLONITE_SYSTEMS_MESH_RENDER_SYSTEM_H
#define CYCLONITE_SYSTEMS_MESH_RENDER_SYSTEM_H

#include "components/mesh.h"
#include "components/transform.h"
#include "core/hashTable.h"
#include "core/resourceSharedRef.h"
#include "material.h"
#include "geometry.h"
#include "root.h"

namespace cyclonite::systems {
class MeshRenderSystem
{
public:
    MeshRenderSystem() = default;

    // TODO:: init swapchain

    template<size_t ExecutionStage, typename Config>
    auto run(Root<Config>& root,
             std::shared_future<void>& prevStageFutures,
             core::ResourceSharedRef const& sceneRef) -> std::future<void>;

private:
    static constexpr size_t max_draw_batch_count_v = 16 * 1024;
    static constexpr size_t max_draw_command_per_buffer_v = 16 * 1024;

    // keys:
    // 1. scene id
    // 2. pipeline id
    // 3. frame index
    // 4. is indexed draw command

    core::StaticHashTable<core::ResourceSharedRef, max_draw_batch_count_v, uint64_t, uint64_t, uint8_t, bool>
      drawBatchBufferStagings_;

    core::StaticHashTable<core::ResourceSharedRef, max_draw_batch_count_v, uint64_t, uint64_t, uint8_t, bool>
      drawBatchBuffer_;

    // key:
    // 1. geometryId
    // 2. stagingId
    core::StaticHashTable<uint16_t, max_draw_command_per_buffer_v, uint64_t, uint64_t> geometryIdToCommandIndex_;

    uint64_t currentFrameNumber_;
    uint8_t swapChainLength_;
};

template<size_t ExecutionStage, typename Config>
auto MeshRenderSystem::run(Root<Config>& root,
                           std::shared_future<void>& prevStageFutures,
                           core::ResourceSharedRef const& sceneRef) -> std::future<void>
{
    if constexpr (ExecutionStage == metrix::value_cast(SystemUpdateStageList::LATE_UPDATE)) {
        if (prevStageFutures.valid()) {
            prevStageFutures.get();
        }

        auto frameIndex = static_cast<uint8_t>(currentFrameNumber_ % swapChainLength_);

        auto sceneKey = static_cast<uint64_t>(sceneRef.id());

        auto& ecs = root.getSceneEntities(sceneRef);

        auto view = ecs.template getView<components::Transform, components::Mesh>();

        for (auto&& [entity, transform, mesh] : view) {
            for (auto&& [geometryRef, materialRef] : mesh.subMeshes()) {
                auto& material = materialRef.template as<Material>();
                auto pipelineKey = static_cast<uint64_t>(material.pipeline().id());

                auto& geometry = geometryRef.template as<Geometry>();
                auto isIndexed = geometry.isIndexed();

                // TODO::
            }
        }
    }

    auto promise = std::promise<void>();
    auto future = promise.get_future();

    promise.set_value();
}
}

#endif // CYCLONITE_SYSTEMS_MESH_RENDER_SYSTEM_H
