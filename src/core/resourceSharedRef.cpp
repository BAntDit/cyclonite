//
// Created by anton on 6/15/25.
//

#include "resourceSharedRef.h"

#include <utility>

#include "resourceManager.h"
#include "resourceUniqueRef.h"

namespace cyclonite::core {
ResourceSharedRef::ResourceSharedRef(ResourceBase* resource)
  : id_{}
  , resource_{ nullptr }
{
    if (resource->retain() > 0) {
        resource_ = resource;
        id_ = resource->resourceId();
    }
}

ResourceSharedRef::ResourceSharedRef(ResourceUniqueRef&& uniqueRef) noexcept
  : id_{ std::exchange(uniqueRef.id_, core::ResourceId{}) }
  , resource_{ std::exchange(uniqueRef.resource_, nullptr) }
{
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
  : id_{ std::exchange(ref.id_, core::ResourceId{}) }
  , resource_{ std::exchange(ref.resource_, nullptr) }
{
}

auto ResourceSharedRef::valid() const -> bool
{
    return resource_ != nullptr && resource_->resourceManager_ != nullptr &&
           resource_->resourceManager_->isResourceValid(id_);
}

auto ResourceSharedRef::typeIndex() const -> uint8_t
{
    return (resource_ == nullptr) ? std::numeric_limits<uint8_t>::max()
                                  : resource_->resourceManager_->resourceTypeIndex(id_);
}

ResourceSharedRef::~ResourceSharedRef()
{
    if (valid()) {
        release();
    }
    id_ = ResourceId{};
    resource_ = nullptr;
}

auto ResourceSharedRef::operator=(ResourceSharedRef const& rhs) -> ResourceSharedRef&
{
    if (valid()) {
        release();
    }

    id_ = rhs.id_;
    resource_ = rhs.resource_;

    if (valid()) {
        retain();
    }

    return *this;
}

auto ResourceSharedRef::operator=(ResourceSharedRef&& rhs) noexcept -> ResourceSharedRef&
{
    if (valid()) {
        release();
    }

    id_ = std::exchange(rhs.id_, core::ResourceId{});
    resource_ = std::exchange(rhs.resource_, nullptr);

    return *this;
}

auto ResourceSharedRef::operator=(ResourceUniqueRef&& rhs) noexcept -> ResourceSharedRef&
{
    if (valid()) {
        release();
    }

    id_ = std::exchange(rhs.id_, core::ResourceId{});
    resource_ = std::exchange(rhs.resource_, nullptr);

    return *this;
}
}
