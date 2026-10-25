//
// Created by anton on 10/4/26.
//

#ifndef CYCLONITE_GEOMETRY_MANAGEMER_H
#define CYCLONITE_GEOMETRY_MANAGEMER_H

#include "core/arena.h"
#include "core/resourceSharedRef.h"
#include "gfx/common.h"
#include "gfx/geometryIndicesAllocation.h"
#include <concepts>
#include <list>
#include <mutex>
#include <new>

namespace cyclonite {
class Geometry;

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

template<typename T>
concept TransferSystemConcept = requires(T& t, core::ResourceSharedRef const& r, size_t s) {
    { t.commitTransferTask(r, r, s, s, s) } -> std::same_as<void>;
};
}

class GeometryManager
{
    friend class Geometry;

public:
    template<internal::TransferSystemConcept TransferSystemType>
    explicit GeometryManager(TransferSystemType& transferSystem);

    void init(core::ResourceSharedRef const& deviceRef, uint32_t initialIndexArenaCapacity);

    [[nodiscard]] auto device() const -> core::ResourceSharedRef const& { return deviceRef_; }

    [[nodiscard]] auto device() -> core::ResourceSharedRef& { return deviceRef_; }

    [[nodiscard]] auto options() const -> GeometryOptions const& { return options_; }

    [[nodiscard]] auto options() -> GeometryOptions& { return options_; }

private:
    [[nodiscard]] auto allocateIndices(uint32_t count, gfx::IndexType type) -> gfx::GeometryIndicesAllocation;

    void commitGpuData(core::ResourceSharedRef const& stagingRef,
                       core::ResourceSharedRef const& gpuResourceRef,
                       size_t offset,
                       size_t size);

    template<internal::TransferSystemConcept TransferSystemType>
    static void transferTaskCommit(void* transferSystemPtr,
                                   core::ResourceSharedRef const& stagingRef,
                                   core::ResourceSharedRef const& gpuResourceRef,
                                   size_t offset,
                                   size_t size);

    using transfer_task_commit_invoke_f = void (*)(void* transferSystemPtr,
                                                   core::ResourceSharedRef const& stagingRef,
                                                   core::ResourceSharedRef const& gpuResourceRef,
                                                   size_t offset,
                                                   size_t size);

    std::list<internal::IndexArena> index32ArenaList_;
    std::list<internal::IndexArena> index16ArenaList_;
    core::ResourceSharedRef deviceRef_;
    void* transferSystemPtr_;
    transfer_task_commit_invoke_f transferTaskInvoke_;
    uint32_t initialIndexArenaCapacity_;
    GeometryOptions options_;
};

template<internal::TransferSystemConcept TransferSystemType>
/*static*/ void GeometryManager::transferTaskCommit(void* transferSystemPtr,
                                                    core::ResourceSharedRef const& stagingRef,
                                                    core::ResourceSharedRef const& gpuResourceRef,
                                                    size_t offset,
                                                    size_t size)
{
    auto* transferSystem = std::launder(reinterpret_cast<TransferSystemType*>(transferSystemPtr));
    transferSystem->commitTransferTask(stagingRef, gpuResourceRef, 0, offset, size);
}

template<internal::TransferSystemConcept TransferSystemType>
GeometryManager::GeometryManager(TransferSystemType& transferSystem)
  : index32ArenaList_{}
  , index16ArenaList_{}
  , deviceRef_{}
  , transferSystemPtr_{ &transferSystem }
  , transferTaskInvoke_{ &transferTaskCommit<TransferSystemType> }
  , initialIndexArenaCapacity_{ 0 }
{
    options_.flags.set(GeometryOptionFlags::ComputeInFlightTBN, GeometryOptionFlags::CompressSkinningAttributes);
    options_.flags.reset(GeometryOptionFlags::KeepsOriginalAssetLayout);
}
}

#endif // CYCLONITE_GEOMETRY_MANAGEMER_H