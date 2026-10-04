//
// Created by anton on 10/4/26.
//

#include "geometryManagementSystem.h"
#include "gfx/device.h"

namespace cyclonite::systems {
internal::IndexArena::IndexArena(GeometryManagementSystem& geometrySystem, size_t count, gfx::IndexType indexType)
  : core::Arena{ count * (indexType == gfx::IndexType::TYPE_UINT16 ? sizeof(uint16_t) : sizeof(uint32_t)) }
  , lock_{}
  , geometrySystem_{ &geometrySystem }
  , indexBufferRef_{}
  , indexType_{ indexType }
{
    auto& device = geometrySystem.device().as<gfx::Device>();

    auto allocationFlags = gfx::GpuMemoryAllocationFlagBits{};
    allocationFlags.set(gfx::GpuMemoryAllocationFlags::DEDICATED_MEMORY);

    auto usageFlags = gfx::BufferUsageFlagBits{};
    usageFlags.set(gfx::BufferUsageFlags::INDEX_BUFFER, gfx::BufferUsageFlags::TRANSFER_DST);

    indexBufferRef_ = device.createBuffer(allocationFlags, usageFlags, capacity());
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

void GeometryManagementSystem::init(core::ResourceSharedRef const& deviceRef, uint32_t initialIndexArenaCapacity)
{
    deviceRef_ = deviceRef;
    initialIndexArenaCapacity_ = initialIndexArenaCapacity;
}

auto GeometryManagementSystem::allocateIndices(uint32_t count, gfx::IndexType type) -> gfx::GeometryIndicesAllocation
{
    auto allocation = gfx::GeometryIndicesAllocation{};

    auto& arenaList = (type == gfx::IndexType::TYPE_UINT16) ? index16ArenaList_ : index32ArenaList_;

    // from back to end, becuase last arena should has more free memory
    for (auto it = arenaList.rbegin(); it != arenaList.rend(); it++) {
        auto& arena = *it;

        if ((allocation = arena.alloc(count)).valid())
            break;
    }

    if (!allocation.valid()) {
        auto capacity = std::max(count, initialIndexArenaCapacity_);
        auto& arena = arenaList.emplace_back(*this, capacity, type);

        if (!(allocation = arena.alloc(count)).valid()) {
            throw std::runtime_error("geometry system is running out of indices memeory");
        }
    }

    return allocation;
}
}
