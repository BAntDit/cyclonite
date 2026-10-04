//
// Created by anton on 10/4/26.
//

#include "geometryManagementSystem.h"
#include "gfx/device.h"

namespace cyclonite::systems {
internal::IndexArena::IndexArena(GeometryManagementSystem& geometryIndicesManager,
                                 size_t capacity,
                                 gfx::IndexType indexType)
  : core::Arena{ capacity }
  , lock_{}
  , geometrySystem_{ &geometryIndicesManager }
  , indexBufferRef_{}
  , indexType_{ indexType }
{
    auto& device = geometryIndicesManager.device().as<gfx::Device>();

    auto byteSize =
      (indexType_ == gfx::IndexType::TYPE_UINT16) ? capacity * sizeof(uint16_t) : capacity * sizeof(uint32_t);

    auto allocationFlags = gfx::GpuMemoryAllocationFlagBits{};
    allocationFlags.set(gfx::GpuMemoryAllocationFlags::DEDICATED_MEMORY);

    auto usageFlags = gfx::BufferUsageFlagBits{};
    usageFlags.set(gfx::BufferUsageFlags::INDEX_BUFFER, gfx::BufferUsageFlags::TRANSFER_DST);

    indexBufferRef_ = device.createBuffer(allocationFlags, usageFlags, byteSize);
}

auto internal::IndexArena::alloc(uint32_t indexCount) -> gfx::GeometryIndicesAllocation
{
    auto requiredSize =
      indexType_ == gfx::IndexType::TYPE_UINT16 ? indexCount * sizeof(uint16_t) : indexCount * sizeof(uint32_t);

    auto alignment = indexType_ == gfx::IndexType::TYPE_UINT16 ? sizeof(uint16_t) : sizeof(uint32_t);

    auto allocation = gfx::GeometryIndicesAllocation{};
    {
        auto lockGuard = std::lock_guard{ lock_ };

        auto allocInfo = Arena::alloc(requiredSize, alignment, alignment);
        assert((allocInfo.rangeOffset % alignment) == 0);

        allocation.firstIndex_ = static_cast<uint32_t>(allocInfo.rangeOffset / alignment);
        allocation.indexCount_ = indexCount;
        allocation.indexType_ = indexType_;
        allocation.arena_ = this;
        allocation.indexBufferRef_ = indexBufferRef_;
    }

    return allocation;
}

void internal::IndexArena::free(uint32_t firstIndex, uint32_t indexCount)
{
    auto offset =
      (indexType_ == gfx::IndexType::TYPE_UINT16) ? firstIndex * sizeof(uint16_t) : firstIndex * sizeof(uint32_t);
    auto size =
      (indexType_ == gfx::IndexType::TYPE_UINT16) ? indexCount * sizeof(uint16_t) : indexCount * sizeof(uint32_t);

    Arena::free(offset, size);
}

auto GeometryManagementSystem::allocateIndices(uint32_t count, gfx::IndexType type) -> gfx::GeometryIndicesAllocation
{
    auto& arenaList = (type == gfx::IndexType::TYPE_UINT16) ? index16ArenaList_ : index32ArenaList_;

    if (arenaList.empty() == 0) {
        // TODO:: capacity
        // arenaList.emplace_back()
    }
    // TODO::
}
}
