//
// Created by anton on 6/13/25.
//

#ifndef GFX_RESOURCE_BASE_H
#define GFX_RESOURCE_BASE_H

#include "resourceId.h"
#include <atomic>
#include <type_traits>

namespace cyclonite::core {
class ResourceManagerBase;
class ResourceSharedRef;
class ResourceUniqueRef;
class ResourceWeakRef;

class ResourceBase
{
    friend class ResourceSharedRef;
    friend class ResourceUniqueRef;
    friend class ResourceManagerBase;

public:
    ResourceBase(ResourceManagerBase* resourceManager, ResourceId resourceId, bool deferredRelease);

    ResourceBase(ResourceBase const&) = delete;
    ResourceBase(ResourceBase&&) = delete;

    auto operator=(ResourceBase const&) -> ResourceBase& = delete;
    auto operator=(ResourceBase&&) -> ResourceBase& = delete;

    auto retain() -> uint64_t;

    auto release() -> uint64_t;

    [[nodiscard]] auto refCount() const -> uint64_t { return refCount_.load(std::memory_order_acquire); }

    [[nodiscard]] auto resourceId() const -> ResourceId { return resourceId_; }

    template<typename T>
    [[nodiscard]] auto as() -> T&
        requires(std::is_base_of_v<ResourceBase, T>)
    {
        return static_cast<T&>(*this);
    }

    template<typename T>
    [[nodiscard]] auto as() const -> T const&
        requires(std::is_base_of_v<ResourceBase, T>)
    {
        return static_cast<T const&>(*this);
    }

    [[nodiscard]] auto resourceManager() -> ResourceManagerBase& { return *resourceManager_; }

    [[nodiscard]] auto resourceManager() const -> ResourceManagerBase const& { return *resourceManager_; }

protected:
    [[nodiscard]] auto resourceBase() -> ResourceBase* { return this; }

private:
    std::atomic<uint64_t> refCount_;
    ResourceManagerBase* resourceManager_;
    ResourceId resourceId_;
    bool deferredRelease_;
};
}

#endif // GFX_RESOURCE_BASE_H
