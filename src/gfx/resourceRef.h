//
// Created by anton on 6/14/25.
//

#ifndef GFX_RESOURCE_REF_H
#define GFX_RESOURCE_REF_H

#include "resourceBase.h"
#include "resourceId.h"
#include "resourceTypeList.h"
#include <concepts>

namespace cyclonite::gfx {
template<typename T>
concept ResourceConcept = resource_type_list_t::has_type<T>::value && std::derived_from<T, ResourceBase>;

class ResourceManager;

class ResourceRef
{
public:
    friend class ResourceManager;

    ResourceRef() = default;

    [[nodiscard]] auto id() const -> ResourceId { return id_; }

    [[nodiscard]] auto valid() const -> bool;

    [[nodiscard]] auto refCount() const -> uint64_t { return resource_->refCount(); }

    auto retain() -> uint64_t { return resource_->retain(); }

    auto release() -> uint64_t { return resource_->release(); }

    [[nodiscard]] auto resourceBase() const -> ResourceBase* { return resource_; }

    template<ResourceConcept R>
    [[nodiscard]] auto as() const -> R const&;

    template<ResourceConcept R>
    [[nodiscard]] auto as() -> R&;

private:
    ResourceRef(ResourceId id, ResourceBase* resource);

    ResourceId id_;
    ResourceBase* resource_;
};

template<ResourceConcept R>
auto ResourceRef::as() const -> R const&
{
    return static_cast<R const&>(*resource_);
}

template<ResourceConcept R>
auto ResourceRef::as() -> R&
{
    return static_cast<R&>(*resource_);
}
}

#endif // GFX_RESOURCE_REF_H
