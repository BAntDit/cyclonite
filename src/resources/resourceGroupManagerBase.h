//
// Created by anton on 4/14/26.
//

#ifndef CYCLONITE_RESOURCES_RESOURCE_GROUP_MANAGER_BASE_H
#define CYCLONITE_RESOURCES_RESOURCE_GROUP_MANAGER_BASE_H

#include "event.h"

namespace cyclonite::resources {
class ResourceGroupManagerBase
{
public:
    virtual ~ResourceGroupManagerBase() = default;

    Event<uint32_t, std::string_view> resourceAdded;
    Event<uint32_t, std::string_view> resourceGoingToBeDeleted;
    Event<uint32_t, std::string_view> resourceLoadingStart;
    Event<uint32_t, std::string_view> resourcePreparingStart;
    Event<uint32_t, std::string_view> resourcePrepared;
    Event<uint32_t, std::string_view> resourceLoaded;
};
}

#endif // CYCLONITE_RESOURCES_RESOURCE_GROUP_MANAGER_BASE_H
