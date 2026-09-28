
#include "geometryIndicesManager.h"
#include "gfx/buffer.h"
#include "gfx/device.h"
#include "gfx/queueSubmissionRecorder.h"

namespace cyclonite::gfx {
namespace internal {
struct IndicesTransferJob
{
    gfx::Device* device;
    core::ResourceSharedRef indexBufferRef;
    core::ResourceSharedRef stagingRef;

    void operator()()
    {
        auto& submissionManager = device->queueSubmissionManager();
        auto poolFlags = gfx::CommandPoolFlagBits{};
        poolFlags.set(gfx::CommandPoolFlags::TRANSIENT);

        auto submissionRef = submissionManager.acquireQueueSubmission(
          multithreading::Purpose::Transfer, poolFlags, gfx::default_transfer_submission_priority_v);

        auto& transferSubmission = submissionRef.as<gfx::QueueSubmission>();

        // TDOO::
    }
};
}

IndicesAllocation::~IndicesAllocation()
{
    auto lockGuard = std::lock_guard{ lock_ };
    if (arena_ != nullptr) {
        arena_->free(firstIndex_, indexCount_);
        arena_ = nullptr;
    }
}

auto IndicesAllocation::lock() -> void*
{
    auto lockGuard = std::lock_guard{ lock_ };
    assert(arena_ != nullptr);

    if (stagingBufferRef_.valid()) {
        throw std::runtime_error("attempt to lock indices twice");
    }

    auto allocationFlags = gfx::GpuMemoryAllocationFlagBits{};
    allocationFlags.set(gfx::GpuMemoryAllocationFlags::HOST_ACCESS_SEQUENTIAL_WRITE);

    auto usageFlags = gfx::BufferUsageFlagBits{};
    usageFlags.set(gfx::BufferUsageFlags::TRANSFER_SRC);

    auto bytesPerIndex = indexType_ == gfx::IndexType::TYPE_UINT16 ? sizeof(uint16_t) : sizeof(uint32_t);

    auto& manager = arena_->indicesManager();

    stagingBufferRef_ = manager.device().createBuffer(allocationFlags, usageFlags, indexCount_ * bytesPerIndex);

    assert(stagingBufferRef_.valid());
    auto& staging = stagingBufferRef_.template as<gfx::Buffer>();

    return staging.map();
}

void IndicesAllocation::unlock()
{
    auto lockGuard = std::lock_guard{ lock_ };
    assert(arena_ != nullptr);

    assert(stagingBufferRef_.valid());
    auto& staging = stagingBufferRef_.template as<gfx::Buffer>();

    staging.unmap();

    auto& manager = arena_->indicesManager();
    auto& device = manager.device();

    auto& submissionManager = device.queueSubmissionManager();

    auto poolFlags = gfx::CommandPoolFlagBits{};
    poolFlags.set(gfx::CommandPoolFlags::TRANSIENT);

    auto submissionRef = submissionManager.acquireQueueSubmission(
      multithreading::Purpose::Transfer, poolFlags, gfx::default_transfer_submission_priority_v);

    // TODO::
}

namespace internal {
IndexArena::IndexArena(GeometryIndicesManager& geometryIndicesManager, size_t capacity, gfx::IndexType indexType)
  : core::Arena{ capacity }
  , indicesManager_{ &geometryIndicesManager }
  , indexBuffer_{}
  , indexType_{ indexType }
{
    auto& device = geometryIndicesManager.device();

    auto allocationFlags = gfx::GpuMemoryAllocationFlagBits{};
    allocationFlags.set(gfx::GpuMemoryAllocationFlags::DEDICATED_MEMORY);

    auto usageFlags = gfx::BufferUsageFlagBits{};
    usageFlags.set(gfx::BufferUsageFlags::INDEX_BUFFER, gfx::BufferUsageFlags::TRANSFER_DST);

    indexBuffer_ = device.createBuffer(allocationFlags, usageFlags, capacity);
}

auto IndexArena::alloc(uint32_t indexCount) -> IndicesAllocation
{
    // TODO::
}
} // internal
}
