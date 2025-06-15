//
// Created by anton on 6/13/25.
//

#ifndef GFX_RESOURCE_BASE_H
#define GFX_RESOURCE_BASE_H

#include "common.h"
#include "resourceId.h"
#include <atomic>
#include <concepts>
#include <cstdint>

namespace cyclonite::gfx {
class ResourceManager;

class ResourceBase
{
public:
    ResourceBase(ResourceManager* resourceManager, ResourceId resourceId, bool deferredRelease);

    ResourceBase(ResourceBase const&) = delete;
    ResourceBase(ResourceBase&&) = delete;

    auto operator=(ResourceBase const&) -> ResourceBase& = delete;
    auto operator=(ResourceBase&&) -> ResourceBase& = delete;

    auto retain() -> uint64_t;

    auto release() -> uint64_t;

    [[nodiscard]] auto refCount() const -> uint64_t { return refCount_.load(std::memory_order_acquire); }

    [[nodiscard]] auto resourceId() const -> ResourceId { return resourceId_; }

    [[nodiscard]] auto valid() const -> bool;

    template<typename T>
    [[nodiscard]] auto as() -> T& requires(std::derived_from<type_traits::platform_implementation_t<T>, ResourceBase>) {
                                      return static_cast<T&>(*this);
                                  }

    template<typename T>
    [[nodiscard]] auto as() const -> T const&
        requires(std::derived_from<type_traits::platform_implementation_t<T>, ResourceBase>)
    {
        return static_cast<T const&>(*this);
    }

private:
    std::atomic<uint64_t> refCount_;
    ResourceManager* resourceManager_;
    ResourceId resourceId_;
    bool deferredRelease_;
};
}

#endif // GFX_RESOURCE_BASE_H
