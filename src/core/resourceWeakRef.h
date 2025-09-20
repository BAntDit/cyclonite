//
// Created by anton on 8/25/25.
//

#ifndef CYCLONITE_RESOURCE_WEAK_REF_H
#define CYCLONITE_RESOURCE_WEAK_REF_H

#include "resourceSharedRef.h"

namespace cyclonite::core {
class ResourceWeakRef
{
public:
    ResourceWeakRef() = default;

    ResourceWeakRef(ResourceSharedRef const& resourceRef);

    ResourceWeakRef(ResourceWeakRef const& weakRef) = default;

    ResourceWeakRef(ResourceWeakRef&& weakRef) = default;

    ~ResourceWeakRef() = default;

    auto operator=(ResourceWeakRef const& rhs) -> ResourceWeakRef& = default;

    auto operator=(ResourceWeakRef&& rhs) -> ResourceWeakRef& = default;

    auto lock() -> ResourceSharedRef;

    [[nodiscard]] auto useCount() const -> uint32_t { return ResourceSharedRef{ id_, resource_ }.refCount(); }

    [[nodiscard]] auto expired() const -> bool { return ResourceSharedRef{ id_, resource_ }.valid(); }

private:
    ResourceId id_;
    ResourceBase* resource_;
};
}

#endif // CYCLONITE_RESOURCE_WEAK_REF_H
