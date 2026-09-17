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
template<typename Config>
class ResourceGroupManager : public ResourceGroupManagerBase
{
public:
    explicit ResourceGroupManager(Root<Config>& root)
      : ResourceGroupManagerBase{}
      , resourceGroups_{}
      , root_{ &root }
    {
    }

    [[nodiscard]] auto addResourceGroup() -> uint32_t;

    auto load(uint32_t groupId, std::wstring_view location) -> std::future<void>;

    template<CustomSourceConcept CustomSource>
    auto load(uint32_t groupId, CustomSource&& customSource) -> std::future<void>
        requires std::is_rvalue_reference_v<CustomSource>;

    auto prepare(uint32_t groupId, core::ResourceSharedRef const& deviceRef) -> std::future<void>;

    template<ManagedResourceConcept R, typename... Args>
    auto addResource(uint32_t groupId, Args&&... args) -> core::ResourceSharedRef;

    void releaseResource(uint32_t groupId, boost::uuids::uuid const& uuid);

    void releaseGroup(uint32_t groupId);

    [[nodiscard]] auto getResource(uint32_t groupId, std::string_view name) const -> core::ResourceSharedRef;

private:
    static std::atomic<uint32_t> nextResourceGroupId;

    std::unordered_map<uint32_t, std::shared_ptr<ResourceGroup<Config>>> resourceGroups_;
    Root<Config>* root_;
};

template<typename Config>
/*static */ std::atomic<uint32_t> ResourceGroupManager<Config>::nextResourceGroupId = 0;

template<typename Config>
auto ResourceGroupManager<Config>::addResourceGroup() -> uint32_t
{
    auto id = nextResourceGroupId.fetch_add(1, std::memory_order_acq_rel);
    auto resourceGroupNew = std::make_shared<ResourceGroup<Config>>(*root_, this, id);
    resourceGroups_.emplace(id, std::move(resourceGroupNew));

    return id;
}

template<typename Config>
auto ResourceGroupManager<Config>::load(uint32_t groupId, std::wstring_view location) -> std::future<void>
{
    auto it = resourceGroups_.find(groupId);
    if (it == resourceGroups_.end()) {
        throw std::runtime_error("Resource group does not exist");
    }

    auto& [_, group] = *it;
    return group->load(location);
}

template<typename Config>
template<CustomSourceConcept CustomSource>
auto ResourceGroupManager<Config>::load(uint32_t groupId, CustomSource&& customSource) -> std::future<void>
    requires std::is_rvalue_reference_v<CustomSource>
{
    auto it = resourceGroups_.find(groupId);
    if (it == resourceGroups_.end()) {
        throw std::runtime_error("Resource group does not exist");
    }

    auto& [_, group] = *it;
    return group->load(std::forward<CustomSource>(customSource));
}

template<typename Config>
auto ResourceGroupManager<Config>::prepare(uint32_t groupId,
                                           core::ResourceSharedRef const& deviceRef) -> std::future<void>
{
    auto it = resourceGroups_.find(groupId);
    if (it == resourceGroups_.end()) {
        throw std::runtime_error("Resource group does not exist");
    }

    auto& [_, group] = *it;
    return group->prepare(deviceRef);
}

template<typename Config>
template<ManagedResourceConcept R, typename... Args>
auto ResourceGroupManager<Config>::addResource(uint32_t groupId, Args&&... args) -> core::ResourceSharedRef
{
    auto it = resourceGroups_.find(groupId);
    if (it == resourceGroups_.end()) {
        throw std::runtime_error("Resource group does not exist");
    }

    auto& [_, group] = *it;
    return core::ResourceSharedRef{ group->template addResource<R>(std::forward<Args>(args)...) };
}

template<typename Config>
void ResourceGroupManager<Config>::releaseResource(uint32_t groupId, boost::uuids::uuid const& uuid)
{
    auto it = resourceGroups_.find(groupId);
    if (it == resourceGroups_.end()) {
        throw std::runtime_error("Resource group does not exist");
    }

    auto& [_, group] = *it;
    group->releaseResource(uuid);
}

template<typename Config>
void ResourceGroupManager<Config>::releaseGroup(uint32_t groupId)
{
    auto it = resourceGroups_.find(groupId);
    if (it == resourceGroups_.end()) {
        throw std::runtime_error("Resource group does not exist");
    }

    auto& [_, group] = *it;
    group->releaseAll();
}

template<typename Config>
auto ResourceGroupManager<Config>::getResource(uint32_t groupId, std::string_view name) const -> core::ResourceSharedRef
{
    auto it = resourceGroups_.find(groupId);
    if (it == resourceGroups_.end()) {
        throw std::runtime_error("Resource group does not exist");
    }

    auto& [_, group] = *it;
    return group->getResource(name);
}
}

#endif // CYCLONITE_RESOURCES_RESOURCE_GROUP_MANAGER_H