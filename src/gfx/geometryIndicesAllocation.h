//
// Created by anton on 10/4/26.
//

#ifndef CYCLONITE_GFX_GEOMETRY_INDICES_ALLOCATION_H
#define CYCLONITE_GFX_GEOMETRY_INDICES_ALLOCATION_H

#include "core/resourceSharedRef.h"
#include "gfx/common.h"

namespace cyclonite::systems::internal {
class IndexArena;
}

namespace cyclonite::gfx {
class GeometryIndicesAllocation
{
    friend class systems::internal::IndexArena;

public:
    GeometryIndicesAllocation() = default;

    GeometryIndicesAllocation(GeometryIndicesAllocation const&) = delete;

    GeometryIndicesAllocation(GeometryIndicesAllocation&&) = default;

    ~GeometryIndicesAllocation();

    auto operator=(GeometryIndicesAllocation const&) -> GeometryIndicesAllocation& = delete;

    auto operator=(GeometryIndicesAllocation&&) -> GeometryIndicesAllocation& = default;

    [[nodiscard]] auto indexBuffer() const -> core::ResourceSharedRef const& { return indexBufferRef_; }

    [[nodiscard]] auto firstIndex() const -> uint32_t { return firstIndex_; }

    [[nodiscard]] auto indexCount() const -> uint32_t { return indexCount_; }

    [[nodiscard]] auto valid() const -> bool { return arena_ != nullptr && indexCount_ > 0 && indexBufferRef_.valid(); }

private:
    core::ResourceSharedRef indexBufferRef_;
    core::ResourceSharedRef stagingBufferRef_;
    systems::internal::IndexArena* arena_;
    uint32_t firstIndex_;
    uint32_t indexCount_;
    gfx::IndexType indexType_;
};
}

#endif // CYCLONITE_GFX_GEOMETRY_INDICES_ALLOCATION_H