//
// Created by anton on 6/14/25.
//

#ifndef GFX_RESOURCE_SHARED_REF_H
#define GFX_RESOURCE_SHARED_REF_H

#include "resourceBase.h"
#include "resourceId.h"

namespace cyclonite::core {
class ResourceManagerBase;
class ResourceWeakRef;
class ResourceUniqueRef;
class EnableRefFromThis;

class ResourceSharedRef
{
public:
    friend class ResourceManagerBase;
    friend class ResourceWeakRef;
    friend class EnableRefFromThis;

    ResourceSharedRef() = default;

    explicit ResourceSharedRef(ResourceUniqueRef&& uniqueRef) noexcept;

    ResourceSharedRef(ResourceSharedRef const& ref);

    ResourceSharedRef(ResourceSharedRef&& ref) noexcept;

    ~ResourceSharedRef();

    auto operator=(ResourceSharedRef const& rhs) -> ResourceSharedRef&;

    auto operator=(ResourceSharedRef&& rhs) noexcept -> ResourceSharedRef&;

    auto operator=(ResourceUniqueRef&& rhs) noexcept -> ResourceSharedRef&;

    [[nodiscard]] auto id() const -> ResourceId { return id_; }

    [[nodiscard]] auto valid() const -> bool;

    [[nodiscard]] auto refCount() const -> uint64_t { return resource_->refCount(); }

    [[nodiscard]] auto resourceBase() const -> ResourceBase* { return resource_; }

    template<typename R>
    [[nodiscard]] auto as() const -> R const&
    {
        return resource_->as<R>();
    }

    template<typename R>
    [[nodiscard]] auto as() -> R&
    {
        return resource_->as<R>();
    }

    friend auto makeResourceSharedRefUnsafe(ResourceBase* resource) -> ResourceSharedRef;

private:
    explicit ResourceSharedRef(ResourceBase* resource);

    auto retain() -> uint64_t { return resource_->retain(); }

    auto release() -> uint64_t { return resource_->release(); }

    ResourceSharedRef(ResourceId id, ResourceBase* resource);

    ResourceId id_;
    ResourceBase* resource_;
};

inline auto makeResourceSharedRefUnsafe(ResourceBase* resource) -> ResourceSharedRef
{
    return ResourceSharedRef{ resource };
}
}

#endif // GFX_RESOURCE_SHARED_REF_H
