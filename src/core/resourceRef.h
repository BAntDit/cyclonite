//
// Created by anton on 6/14/25.
//

#ifndef GFX_RESOURCE_REF_H
#define GFX_RESOURCE_REF_H

#include "resourceBase.h"
#include "resourceId.h"

namespace cyclonite::core {
class ResourceManagerBase;

class WeakResourceRef;

class ResourceRef
{
public:
    friend class WeakResourceRef;

    friend class ResourceManagerBase;

    ResourceRef() = default;

    ResourceRef(ResourceRef const& ref);

    ResourceRef(ResourceRef&& ref);

    ~ResourceRef();

    auto operator=(ResourceRef const& rhs) -> ResourceRef&;

    auto operator=(ResourceRef&& rhs) -> ResourceRef&;

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

private:
    auto retain() -> uint64_t { return resource_->retain(); }

    auto release() -> uint64_t { return resource_->release(); }

    ResourceRef(ResourceId id, ResourceBase* resource);

    ResourceId id_;
    ResourceBase* resource_;
};
}

#endif // GFX_RESOURCE_REF_H
