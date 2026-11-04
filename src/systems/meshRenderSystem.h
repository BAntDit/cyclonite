//
// Created by anton on 11/2/26.
//

#ifndef CYCLONITE_SYSTEMS_MESH_RENDER_SYSTEM_H
#define CYCLONITE_SYSTEMS_MESH_RENDER_SYSTEM_H

#include "components/mesh.h"
#include "components/transform.h"
#include "core/hashTable.h"
#include "core/resourceSharedRef.h"
#include "geometry.h"
#include "gfx/drawIndirectCommands.h"
#include "material.h"
#include "root.h"

namespace cyclonite::systems {
class MeshRenderSystem
{
public:
    MeshRenderSystem() = default;

    // TODO:: init frame ring size

    template<size_t ExecutionStage, typename Config>
    auto run(Root<Config>& root, std::shared_future<void>& prevStageFutures, core::ResourceSharedRef const& sceneRef)
      -> std::future<void>;

private:
    static constexpr size_t max_draw_batch_count_v = 16 * 1024;
    static constexpr size_t max_draw_command_per_buffer_v = 16 * 1024;
    static constexpr uint32_t initial_draw_command_count_v = 64;

    core::ResourceSharedRef deviceRef_;

    // keys:
    // 1. scene id
    // 2. pipeline id
    // 3. frame index

    core::StaticHashTable<gfx::DrawIndirectCommands, max_draw_batch_count_v, uint64_t, uint64_t, uint8_t>
      nonIndexedDrawBatchBufferStagings_;

    // TODO:: staging struct: ref, mapped ptr, count

    core::StaticHashTable<core::ResourceSharedRef, max_draw_batch_count_v, uint64_t, uint64_t, uint8_t>
      nonIndexedDrawBatchBuffers_;

    // key:
    // 1. geometryId
    // 2. stagingId
    core::StaticHashTable<uint32_t, max_draw_command_per_buffer_v, uint64_t, uint64_t> geometryIdToCommandIndex_;

    uint64_t currentFrameNumber_;
    uint8_t frameRingSize_;
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

        auto frameIndex = static_cast<uint8_t>(currentFrameNumber_ % frameRingSize_);

        auto sceneKey = static_cast<uint64_t>(sceneRef.id());

        auto& ecs = root.getSceneEntities(sceneRef);

        auto view = ecs.template getView<components::Transform, components::Mesh>();

        for (auto&& [entity, transform, mesh] : view) {
            for (auto&& [geometryRef, materialRef] : mesh.subMeshes()) {
                auto& material = materialRef.template as<Material>();
                auto pipelineKey = static_cast<uint64_t>(material.pipeline().id());

                auto geometryKey = static_cast<uint64_t>(geometryRef.id());
                auto& geometry = geometryRef.template as<Geometry>();
                auto isIndexed = geometry.isIndexed();

                // TODO:: handle per instance and per batch data
                if (isIndexed) {
                    auto stagingIt = nonIndexedDrawBatchBuffers_.find(sceneKey, pipelineKey, frameIndex);

                    if (stagingIt == nonIndexedDrawBatchBuffers_.end()) {
                        auto [it, success] = nonIndexedDrawBatchBufferStagings_.add(
                          gfx::DrawIndirectCommands{ deviceRef_, initial_draw_command_count_v });

                        if (!success) { // skip geometry draw if no memory for batch
                            continue;   // TODO:: mayebe warn it somehow
                        }

                        stagingIt = it;
                    }

                    auto& [_, drawIndirectCommands] = stagingIt;
                    auto stagingBufferRef = drawIndirectCommands.staging();
                    auto stagingKey = static_cast<uint64_t>(stagingBufferRef.id());
                    auto commandIndexIt = geometryIdToCommandIndex_.find(geometryKey, stagingKey);

                    if (commandIndexIt == geometryIdToCommandIndex_.end()) { // if new command
                        auto commandIndexNew = drawIndirectCommands.commandCount();
                        auto [it, success] = geometryIdToCommandIndex_.add(commandIndexNew, geometryKey, stagingKey);
                        if (!success) { // skip geometry draw if no memory for batch
                            continue;
                        }
                        drawIndirectCommands.addCommand(geometry.vertexCount(), 0, 1, 0);
                    } else {
                        auto const& [k, commandIndex] = *commandIndexIt;
                        drawIndirectCommands.instanceCount(commandIndex)++;
                    }
                } else {
                }

                // TODO::
            }
        }
    } else if constexpr (ExecutionStage == metrix::value_cast(SystemUpdateStageList::FRAME_END)) {
        currentFrameNumber_++;
    }

    (void)root;
    (void)sceneRef;
    (void)prevStageFutures;

    auto promise = std::promise<void>();
    auto future = promise.get_future();

    promise.set_value();
}
}

#endif // CYCLONITE_SYSTEMS_MESH_RENDER_SYSTEM_H
