//
// Created by anton on 10/4/26.
//

#ifndef CYCLONITE_GEOMETRY_MANAGEMER_H
#define CYCLONITE_GEOMETRY_MANAGEMER_H

#include "core/arena.h"
#include "core/resourceSharedRef.h"
#include "gfx/common.h"
#include "gfx/geometryIndicesAllocation.h"
#include <list>
#include <mutex>

namespace cyclonite {
template<typename Config>
class Root;
}

namespace cyclonite {
namespace internal {
class IndexArena : protected core::Arena
{
public:
    IndexArena(core::ResourceSharedRef const& deviceRef, size_t count, gfx::IndexType indexType);

    [[nodiscard]] auto alloc(uint32_t indexCount) -> gfx::GeometryIndicesAllocation;

    void free(uint32_t firstIndex, uint32_t indexCount);

    using Arena::capacity;
    using Arena::freeAll;

private:
    std::mutex lock_;
    core::ResourceSharedRef deviceRef_;
    core::ResourceSharedRef indexBufferRef_;
    gfx::IndexType indexType_;
};
}

class GeometryManager
{
public:
    GeometryManager() = default;

    // pass here transfer system template
    void init(core::ResourceSharedRef const& deviceRef, uint32_t initialIndexArenaCapacity);

    [[nodiscard]] auto device() const -> core::ResourceSharedRef const& { return deviceRef_; }

    [[nodiscard]] auto device() -> core::ResourceSharedRef& { return deviceRef_; }

    // ensure geometry
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

#endif // CYCLONITE_GEOMETRY_MANAGEMER_H