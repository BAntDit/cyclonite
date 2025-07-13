//
// Created by anton on 6/15/25.
//

#include "resourceRef.h"
#include "resourceManager.h"

namespace cyclonite::core {
ResourceRef::ResourceRef(ResourceId id, ResourceBase* resource)
  : id_{ id }
  , resource_{ resource } {};

auto ResourceRef::valid() const -> bool
{
    return resource_->resourceManager_->isResourceValid(id_);
}
}
