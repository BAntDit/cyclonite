//
// Created by anton on 8/25/25.
//

#ifndef CYCLONITE_WEAKRESOURCEREF_H
#define CYCLONITE_WEAKRESOURCEREF_H

#include "resourceRef.h"

namespace cyclonite::core {
class WeakResourceRef
{
public:
    WeakResourceRef() = default;

    WeakResourceRef(ResourceRef const& resourceRef);

    WeakResourceRef(WeakResourceRef const& weakRef) = default;

    WeakResourceRef(WeakResourceRef&& weakRef) = default;

    ~WeakResourceRef() = default;

    auto operator=(WeakResourceRef const& rhs) -> WeakResourceRef& = default;

    auto operator=(WeakResourceRef&& rhs) -> WeakResourceRef& = default;

    auto lock() -> ResourceRef;

    [[nodiscard]] auto useCount() const -> uint32_t { return ResourceRef{ id_, resource_ }.refCount(); }

    [[nodiscard]] auto expired() const -> bool { return ResourceRef{ id_, resource_ }.valid(); }

private:
    ResourceId id_;
    ResourceBase* resource_;
};
}

#endif // CYCLONITE_WEAKRESOURCEREF_H
