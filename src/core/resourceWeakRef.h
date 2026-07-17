//
// Created by anton on 8/25/25.
//

#ifndef CYCLONITE_RESOURCE_WEAK_REF_H
#define CYCLONITE_RESOURCE_WEAK_REF_H

#include "resourceSharedRef.h"

namespace cyclonite::core {
class EnableRefFromThis;

class ResourceWeakRef
{
    friend class EnableRefFromThis;

public:
    ResourceWeakRef() = default;

    ResourceWeakRef(ResourceSharedRef const& resourceRef);

    ResourceWeakRef(ResourceWeakRef const& weakRef) = default;

    ResourceWeakRef(ResourceWeakRef&& weakRef) = default;

    ~ResourceWeakRef() = default;

    auto operator=(ResourceWeakRef const& rhs) -> ResourceWeakRef& = default;

    auto operator=(ResourceWeakRef&& rhs) -> ResourceWeakRef& = default;

    auto lock() -> ResourceSharedRef;

    [[nodiscard]] auto useCount() const -> uint64_t;

    [[nodiscard]] auto expired() const -> bool;

    friend auto makeResourceWeakRefUnsafe(ResourceBase* resource) -> ResourceWeakRef;

private:
    explicit ResourceWeakRef(ResourceBase* resource);

    ResourceId id_;
    ResourceBase* resource_;
};

inline auto makeResourceWeakRefUnsafe(ResourceBase* resource) -> ResourceWeakRef
{
    return ResourceWeakRef{ resource };
}
}

#endif // CYCLONITE_RESOURCE_WEAK_REF_H
