//
// Created by anton on 3/8/26.
//

#ifndef CYCLONITE_RESOURCES_RESOURCE_GROUP_MANAGER_H
#define CYCLONITE_RESOURCES_RESOURCE_GROUP_MANAGER_H

#include "resourceGroup.h"
#include <atomic>
#include <cstdint>
#include <memory>
#include <unordered_map>

namespace cyclonite::resources {
template<ManagedResourceConcept... Resources>
class ResourceGroupManager : public ResourceGroupManagerBase
{
public:
    ResourceGroupManager() = default;

    [[nodiscard]] auto addResourceGroup() -> uint32_t;

    auto load(uint32_t groupId, std::wstring_view location) -> std::future<void>;

    template<CustomSourceConcept CustomSource>
    auto load(uint32_t groupId, CustomSource&& customSource) -> std::future<void>
        requires std::is_rvalue_reference_v<CustomSource>;

    // TODO:: remove resource
    // unload

    // prepare

    // add

    // get

private:
    static std::atomic<uint32_t> nextResourceGroupId;

    std::unordered_map<uint32_t, ResourceGroup<Resources...>> resourceGroups_;
};

template<ManagedResourceConcept... Resources>
auto ResourceGroupManager<Resources...>::addResourceGroup() -> uint32_t
{
    auto id = nextResourceGroupId.fetch_add(1, std::memory_order_acq_rel);

    resourceGroups_.emplace(id, ResourceGroup<Resources...>{ this, id });
    return id;
}

template<ManagedResourceConcept... Resources>
auto ResourceGroupManager<Resources...>::load(uint32_t groupId, std::wstring_view location) -> std::future<void>
{
    auto it = resourceGroups_.find(groupId);
    if (it == resourceGroups_.end()) {
        throw std::runtime_error("Resource group does not exist");
    }

    auto& [_, group] = *it;
    return group.load(location);
}

template<ManagedResourceConcept... Resources>
template<CustomSourceConcept CustomSource>
auto ResourceGroupManager<Resources...>::load(uint32_t groupId, CustomSource&& customSource) -> std::future<void>
    requires std::is_rvalue_reference_v<CustomSource>
{
    auto it = resourceGroups_.find(groupId);
    if (it == resourceGroups_.end()) {
        throw std::runtime_error("Resource group does not exist");
    }

    auto& [_, group] = *it;
    return group.load(std::move(customSource));
}
}

#endif // CYCLONITE_RESOURCES_RESOURCE_GROUP_MANAGER_H