//
// Created by anton on 3/9/26.
//

#ifndef CYCLONITE_RESOURCES_RESOURCE_GROUP_H
#define CYCLONITE_RESOURCES_RESOURCE_GROUP_H

#include "core/resourceManager.h"
#include <list>
#include <wstring>

namespace cyclonite::resources
{
template<typename... Resources>
class ResourceGroup
{
public:

private:
    uint32_t id_;
    std::list<std::wstring> locations_;
    core::ResourceManager<Resources...> resourcesLifetimeManager_;
};
}

#endif // CYCLONITE_RESOURCES_RESOURCE_GROUP_H
