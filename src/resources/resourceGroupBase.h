//
// Created by anton on 3/24/26.
//

#ifndef CYCLONITE_RESOURCES_RESOURCE_GROUP_BASE_H
#define CYCLONITE_RESOURCES_RESOURCE_GROUP_BASE_H

#include <string>
#include "managedResourceState.h"

namespace cyclonite::resources {
class ResourceGroupBase
{
public:
    explicit ResourceGroupBase(uint32_t id)
      : id_{ id }
    {
    }

    virtual ~ResourceGroupBase() = default;

    [[nodiscard]] uint32_t id() const { return id_; }

    void notifyResourceStateChange(ManagedResourceState newState,  std::string_view resourceName);

private:
    uint32_t id_;
};
}

#endif // CYCLONITE_RESOURCES_RESOURCE_GROUP_BASE_H