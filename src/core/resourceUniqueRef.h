//
// Created by anton on 9/20/25.
//

#ifndef CYCLONITE_RESOURCE_UNIQUEREF_H
#define CYCLONITE_RESOURCE_UNIQUEREF_H

#include "resourceBase.h"
#include "resourceId.h"

namespace cyclonite::core {
class ResourceSharedRef;

class ResourceUniqueRef
{
    friend class ResourceSharedRef;
    friend class ResourceManagerBase;

public:
    ResourceUniqueRef() = default;

    ResourceUniqueRef(ResourceUniqueRef const&) = delete;

    ResourceUniqueRef(ResourceUniqueRef&& uniqueRef) noexcept;

    ~ResourceUniqueRef();

    auto operator=(ResourceUniqueRef const&) -> ResourceUniqueRef& = delete;

    auto operator=(ResourceUniqueRef&& rhs) noexcept -> ResourceUniqueRef&;

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
    ResourceUniqueRef(ResourceId id, ResourceBase* resource);

    ResourceId id_;
    ResourceBase* resource_;
};
}

#endif // CYCLONITE_RESOURCE_UNIQUEREF_H