//
// Created by anton on 6/13/25.
//

#ifndef GFX_RESOURCE_BASE_H
#define GFX_RESOURCE_BASE_H

#include <atomic>
#include <cstdint>

namespace cyclonite::gfx {
template<typename T>
concept ResourceConcept = requires(T t) {
                              {
                                  t.reset()
                              };
                          };

template<typename Resource>
class ResourceBase
{
public:
    ResourceBase();

    ResourceBase(ResourceBase const&) = delete;
    ResourceBase(ResourceBase&&) = delete;

    auto operator=(ResourceBase const&) -> ResourceBase& = delete;
    auto operator=(ResourceBase&&) -> ResourceBase& = delete;

    auto retain() -> uint64_t;

    auto release() -> uint64_t
        requires(ResourceConcept<Resource>());

    [[nodiscard]] auto refCount() const -> uint64_t { return refCount_.load(std::memory_order_acquire); }

private:
    std::atomic<uint64_t> refCount_;
};
}

#endif // GFX_RESOURCE_BASE_H
