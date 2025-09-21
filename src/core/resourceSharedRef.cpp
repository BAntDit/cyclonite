//
// Created by anton on 6/15/25.
//

#include "resourceSharedRef.h"
#include "resourceManager.h"
#include "resourceUniqueRef.h"

namespace cyclonite::core {
ResourceSharedRef::ResourceSharedRef(ResourceBase* resource)
  : id_{ resource->resourceId() }
  , resource_{ resource }
{
    retain();
}

ResourceSharedRef::ResourceSharedRef(ResourceUniqueRef&& uniqueRef) noexcept
  : id_{ uniqueRef.id_ }
  , resource_{ uniqueRef.resource_ }
{
    uniqueRef.id_ = ResourceId{};
    uniqueRef.resource_ = nullptr;
}

ResourceSharedRef::ResourceSharedRef(ResourceId id, ResourceBase* resource)
  : id_{ id }
  , resource_{ resource } {};

ResourceSharedRef::ResourceSharedRef(ResourceSharedRef const& ref)
  : id_{ ref.id_ }
  , resource_{ ref.resource_ }
{
    if (valid()) {
        retain();
    }
}

ResourceSharedRef::ResourceSharedRef(ResourceSharedRef&& ref) noexcept
  : id_{ ref.id_ }
  , resource_{ ref.resource_ }
{
    ref.id_ = ResourceId{};
    ref.resource_ = nullptr;
}

auto ResourceSharedRef::valid() const -> bool
{
    return resource_ != nullptr && resource_->resourceManager_->isResourceValid(id_);
}

ResourceSharedRef::~ResourceSharedRef()
{
    if (valid()) {
        release();
    }
}

auto ResourceSharedRef::operator=(ResourceSharedRef const& rhs) -> ResourceSharedRef&
{
    id_ = rhs.id_;
    resource_ = rhs.resource_;

    if (valid()) {
        retain();
    }

    return *this;
}

auto ResourceSharedRef::operator=(ResourceSharedRef&& rhs) noexcept -> ResourceSharedRef&
{
    id_ = rhs.id_;
    resource_ = rhs.resource_;

    rhs.id_ = ResourceId{};
    rhs.resource_ = nullptr;

    return *this;
}
}
