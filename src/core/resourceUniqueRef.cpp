//
// Created by anton on 9/21/25.
//

#include "resourceUniqueRef.h"
#include "resourceManager.h"

namespace cyclonite::core {
ResourceUniqueRef::ResourceUniqueRef(ResourceId id, ResourceBase* resource)
  : id_{ id }
  , resource_{ resource }
{
}

ResourceUniqueRef::ResourceUniqueRef(ResourceUniqueRef&& uniqueRef) noexcept
  : id_{ uniqueRef.id_ }
  , resource_{ uniqueRef.resource_ }
{
    uniqueRef.id_ = ResourceId{};
    uniqueRef.resource_ = nullptr;
}

ResourceUniqueRef::~ResourceUniqueRef()
{
    if (valid()) {
        resource_->release();
    }
}

auto ResourceUniqueRef::valid() const -> bool
{
    return resource_ != nullptr && resource_->resourceManager_->isResourceValid(id_);
}

auto ResourceUniqueRef::operator=(ResourceUniqueRef&& rhs) noexcept -> ResourceUniqueRef&
{
    id_ = rhs.id_;
    rhs.id_ = ResourceId{};

    resource_ = rhs.resource_;
    rhs.resource_ = nullptr;

    return *this;
}
}
