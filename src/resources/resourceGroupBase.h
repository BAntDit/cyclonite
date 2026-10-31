//
// Created by anton on 3/24/26.
//

#ifndef CYCLONITE_RESOURCES_RESOURCE_GROUP_BASE_H
#define CYCLONITE_RESOURCES_RESOURCE_GROUP_BASE_H

#include "core/resourceSharedRef.h"
#include "managedResourceState.h"
#include "resourceGroupManagerBase.h"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <string_view>
#include <unordered_map>

namespace cyclonite::resources {
class ResourceGroupBase
{
public:
    explicit ResourceGroupBase(ResourceGroupManagerBase* groupManager, uint32_t id)
      : resourceNameToRef_{}
      , resourceUuidToRef_{}
      , groupManager_{ groupManager }
      , id_{ id }
    {
    }

    virtual ~ResourceGroupBase() = default;

    [[nodiscard]] uint32_t id() const { return id_; }

    [[nodiscard]] auto getResource(boost::uuids::uuid const& uuid) const -> core::ResourceSharedRef;

    [[nodiscard]] auto getResource(std::string_view name) const -> core::ResourceSharedRef;

    [[nodiscard]] auto isExists(boost::uuids::uuid const& uuid) const -> bool;

    [[nodiscard]] auto isExists(std::string_view name) const -> bool;

    void notifyResourceStateChange(ManagedResourceState newState, std::string_view resourceName);

    void notifyResourceAdded(std::string_view resourceName);

protected:
    std::unordered_map<std::string_view, core::ResourceSharedRef> resourceNameToRef_;
    std::unordered_map<std::string, core::ResourceSharedRef> resourceUuidToRef_;

private:
    ResourceGroupManagerBase* groupManager_;
    uint32_t id_;
};
}

#endif // CYCLONITE_RESOURCES_RESOURCE_GROUP_BASE_H
