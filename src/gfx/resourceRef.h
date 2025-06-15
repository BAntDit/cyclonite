//
// Created by anton on 6/14/25.
//

#ifndef GFX_RESOURCE_REF_H
#define GFX_RESOURCE_REF_H

#include "resourceBase.h"
#include "resourceId.h"
#include "resourceTypeList.h"

namespace cyclonite::gfx {
class ResourceManager;

class ResourceRef
{
public:
    friend class ResourceManager;

    [[nodiscard]] auto id() const -> ResourceId { return id_; }

    [[nodiscard]] auto valid() const -> bool;

    [[nodiscard]] auto refCount() const -> uint64_t;

    template<typename T>
    [[nodiscard]] auto as() const -> T const&
        requires(resource_type_list_t::has_type<T>::value);

    template<typename T>
    [[nodiscard]] auto as() -> T&
        requires(resource_type_list_t::has_type<T>::value);

private:
    ResourceRef(ResourceId id, ResourceBase* resource)
      : id_{ id }
      , resource_{ resource } {};

    ResourceId id_;
    ResourceBase* resource_;
};
}

#endif // GFX_RESOURCE_REF_H
