//
// Created by anton on 6/14/25.
//

#ifndef GFX_RESOURCE_REF_H
#define GFX_RESOURCE_REF_H

#include "resourceBase.h"
#include "resourceId.h"

namespace cyclonite::core {
class ResourceManagerBase;

class ResourceRef
{
public:
    friend class ResourceManagerBase;

    ResourceRef() = default;

    [[nodiscard]] auto id() const -> ResourceId { return id_; }

    [[nodiscard]] auto valid() const -> bool;

    [[nodiscard]] auto refCount() const -> uint64_t { return resource_->refCount(); }

    auto retain() -> uint64_t { return resource_->retain(); }

    auto release() -> uint64_t { return resource_->release(); }

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
    ResourceRef(ResourceId id, ResourceBase* resource);

    ResourceId id_;
    ResourceBase* resource_;
};
}

#endif // GFX_RESOURCE_REF_H
