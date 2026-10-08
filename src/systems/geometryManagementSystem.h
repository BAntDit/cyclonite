//
// Created by anton on 10/4/26.
//

#ifndef CYCLONITE_SYSTEMS_GEOMETRY_MANAGEMENT_SYSTEM_H
#define CYCLONITE_SYSTEMS_GEOMETRY_MANAGEMENT_SYSTEM_H

#include "core/arena.h"
#include "core/resourceSharedRef.h"
#include "gfx/common.h"
#include "gfx/geometryIndicesAllocation.h"
#include "stages.h"
#include <future>
#include <list>
#include <metrix/enum.h>
#include <mutex>

namespace cyclonite {
template<typename Config>
class Root;
}

namespace cyclonite::systems {
class GeometryManagementSystem;

namespace internal {
class IndexArena : protected core::Arena
{
public:
    IndexArena(GeometryManagementSystem& geometrySystem, size_t count, gfx::IndexType indexType);

    [[nodiscard]] auto alloc(uint32_t indexCount) -> gfx::GeometryIndicesAllocation;

    void free(uint32_t firstIndex, uint32_t indexCount);

    using Arena::capacity;
    using Arena::freeAll;

private:
    std::mutex lock_;
    GeometryManagementSystem* geometrySystem_;
    core::ResourceSharedRef indexBufferRef_;
    gfx::IndexType indexType_;
};
}

class GeometryManagementSystem
{
public:
    GeometryManagementSystem() = default;

    void init(core::ResourceSharedRef const& deviceRef, uint32_t initialIndexArenaCapacity);

    /*template<size_t ExecutionStage, typename Config>
    auto run(Root<Config>& root,
             std::shared_future<void>& prevStageFutures,
             core::ResourceSharedRef const& sceneRef) -> std::future<void>;*/

    [[nodiscard]] auto device() const -> core::ResourceSharedRef const& { return deviceRef_; }

    [[nodiscard]] auto device() -> core::ResourceSharedRef& { return deviceRef_; }

    // push the current CPU state toward the GPU on the next transfer stage
    // auto commitGeometry(core::ResourceSharedRef geometryRef) -> std::future<void>;

private:
    [[nodiscard]] auto allocateIndices(uint32_t count, gfx::IndexType type) -> gfx::GeometryIndicesAllocation;

    std::list<internal::IndexArena> index32ArenaList_;
    std::list<internal::IndexArena> index16ArenaList_;
    core::ResourceSharedRef deviceRef_;
    uint32_t initialIndexArenaCapacity_;
};
}

#endif // CYCLONITE_SYSTEMS_GEOMETRY_MANAGEMENT_SYSTEM_H