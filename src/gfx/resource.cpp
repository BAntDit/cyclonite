//
// Created by anton on 6/14/25.
//

#include "resource.h"
#include <cassert>

namespace cyclonite::gfx {
template<typename Resource>
ResourceBase<Resource>::ResourceBase()
  : refCount_{ 1 }
{
}

template<typename Resource>
auto ResourceBase<Resource>::retain() -> uint64_t
{
    auto expected = refCount_.load(std::memory_order_relaxed);
    auto desired = expected + 1;
    assert(expected != 0);

    while (!refCount_.compare_exchange_weak(expected, desired, std::memory_order_release, std::memory_order_acquire)) {
        assert(expected != 0);
        desired = expected + 1;
    }

    return desired;
}

template<typename Resource>
auto ResourceBase<Resource>::release() -> uint64_t
    requires(ResourceConcept<Resource>())
{
    auto count = uint32_t{ 0 };

    if (count = refCount_.fetch_sub(1, std::memory_order_acq_rel); count == 1) {
        (static_cast<Resource*>(this))->reset();
    }

    assert(count > 0);
    return count - 1;
}
}
