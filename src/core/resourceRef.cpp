//
// Created by anton on 6/15/25.
//

#include "resourceRef.h"
#include "resourceManager.h"

namespace cyclonite::core {
ResourceRef::ResourceRef(ResourceBase* resource)
  : id_{ resource->resourceId() }
  , resource_{ resource }
{
    retain();
}

ResourceRef::ResourceRef(ResourceId id, ResourceBase* resource)
  : id_{ id }
  , resource_{ resource } {};

ResourceRef::ResourceRef(ResourceRef const& ref)
  : id_{ ref.id_ }
  , resource_{ ref.resource_ }
{
    if (valid()) {
        retain();
    }
}

ResourceRef::ResourceRef(ResourceRef&& ref)
  : id_{ ref.id_ }
  , resource_{ ref.resource_ }
{
    ref.id_ = ResourceId{};
    ref.resource_ = nullptr;
}

auto ResourceRef::valid() const -> bool
{
    return resource_->resourceManager_->isResourceValid(id_);
}

ResourceRef::~ResourceRef()
{
    if (valid()) {
        release();
    }
}

auto ResourceRef::operator=(ResourceRef const& rhs) -> ResourceRef&
{
    id_ = rhs.id_;
    resource_ = rhs.resource_;

    if (valid()) {
        retain();
    }

    return *this;
}

auto ResourceRef::operator=(ResourceRef&& rhs) -> ResourceRef&
{
    id_ = rhs.id_;
    resource_ = rhs.resource_;

    rhs.id_ = ResourceId{};
    rhs.resource_ = nullptr;

    return *this;
}
}
