//
// Created by anton on 3/24/26.
//

#ifndef CYCLONITE_RESOURCES_RESOURCE_GROUP_BASE_H
#define CYCLONITE_RESOURCES_RESOURCE_GROUP_BASE_H

#include "core/resourceSharedRef.h"
#include "managedResourceState.h"
#include "resourceGroupManagerBase.h"
#include <string>

namespace cyclonite::resources {
class ResourceGroupBase
{
public:
    explicit ResourceGroupBase(ResourceGroupManagerBase* groupManager,
                               uint32_t id,
                               core::ResourceSharedRef const& deviceRef)
      : deviceRef_{ deviceRef }
      , groupManager_{ groupManager }
      , id_{ id }
    {
    }

    virtual ~ResourceGroupBase() = default;

    [[nodiscard]] uint32_t id() const { return id_; }

    [[nodiscard]] auto deviceRef() const -> core::ResourceSharedRef const& { return deviceRef_; }

    void notifyResourceStateChange(ManagedResourceState newState, std::string_view resourceName);

    void notifyResourceAdded(std::string_view resourceName);

private:
    core::ResourceSharedRef deviceRef_;
    ResourceGroupManagerBase* groupManager_;
    uint32_t id_;
};
}

#endif // CYCLONITE_RESOURCES_RESOURCE_GROUP_BASE_H