//
// Created by anton on 6/14/25.
//

#include "resourceBase.h"
#include "resourceManager.h"
#include <cassert>

namespace cyclonite::core {
ResourceBase::ResourceBase(ResourceManagerBase* resourceManager, ResourceId resourceId, bool deferredRelease)
  : refCount_{ 1 }
  , resourceManager_{ resourceManager }
  , resourceId_{ resourceId }
  , deferredRelease_{ deferredRelease }
{
}

auto ResourceBase::retain() -> uint64_t
{
    auto expected = refCount_.load(std::memory_order_relaxed);
    auto desired = expected + 1;

    if (expected > 0) {
        while (
          !refCount_.compare_exchange_weak(expected, desired, std::memory_order_acq_rel, std::memory_order_acquire)) {
            if (expected == 0) {
                desired = 0;
                break;
            }

            desired = expected + 1;
        }
    } else {
        desired = 0;
    }

    return desired;
}

auto ResourceBase::release() -> uint64_t
{
    assert(resourceManager_->isResourceValid(resourceId_));

    auto count = uint32_t{ 0 };

    if (count = refCount_.fetch_sub(1, std::memory_order_acq_rel); count == 1) {
        if (deferredRelease_) {
            resourceManager_->releaseResourceDeferred(resourceId_);
        } else {
            resourceManager_->releaseResourceImmediate(resourceId_);
        }
        resourceManager_ = nullptr;
        resourceId_ = ResourceId{};
    }

    assert(count > 0);
    return count - 1;
}
}
