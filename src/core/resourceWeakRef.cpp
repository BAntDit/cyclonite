//
// Created by anton on 8/25/25.
//

#include "resourceWeakRef.h"

namespace cyclonite::core {
ResourceWeakRef::ResourceWeakRef(const ResourceSharedRef& resourceRef)
  : id_{ resourceRef.id_ }
  , resource_{ resourceRef.resource_ }
{
}

auto ResourceWeakRef::lock() -> ResourceSharedRef
{
    auto ref = ResourceSharedRef{ id_, resource_ };
    ref.retain();
    return ref;
}

ResourceWeakRef::ResourceWeakRef(ResourceBase* resource)
  : id_{ resource->resourceId() }
  , resource_{ resource }
{
}

auto ResourceWeakRef::useCount() const -> uint64_t
{
    auto&& ref = ResourceSharedRef{ id_, resource_ };
    auto count = ref.refCount();

    ref.retain();

    return count;
}

auto ResourceWeakRef::expired() const -> bool
{
    auto&& ref = ResourceSharedRef{ id_, resource_ };
    auto valid = ref.valid();

    ref.retain();

    return valid;
}
}
