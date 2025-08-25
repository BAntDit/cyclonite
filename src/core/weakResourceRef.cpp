//
// Created by anton on 8/25/25.
//

#include "weakResourceRef.h"

namespace cyclonite::core {
WeakResourceRef::WeakResourceRef(const cyclonite::core::ResourceRef& resourceRef)
  : id_{ resourceRef.id_ }
  , resource_{ resourceRef.resource_ }
{
}

auto WeakResourceRef::lock() -> ResourceRef
{
    auto ref = ResourceRef{ id_, resource_ };
    ref.retain();
    return ref;
}
}
