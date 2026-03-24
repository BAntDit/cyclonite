//
// Created by anton on 3/8/26.
//

#ifndef CYCLONITE_RESOURCES_RESOURCE_MANAGER_H
#define CYCLONITE_RESOURCES_RESOURCE_MANAGER_H

#include "core/resourceManager.h"

namespace cyclonite::resources {
template<typename... Resources>
class ResourceManager : public core::ResourceManager<Resources...>
{
public:
private:
    // TODO:: move to resource group
    core::ResourceManager<Resources...> resourceLifetimeManager_;
};
}

#endif // CYCLONITE_RESOURCES_RESOURCE_MANAGER_H