//
// Created by anton on 3/9/26.
//

#ifndef CYCLONITE_RESOURCES_RESOURCE_GROUP_H
#define CYCLONITE_RESOURCES_RESOURCE_GROUP_H

#include "core/resourceManager.h"
#include <metrix/type_list.h>

#include "defaultResourceLoader.h"
#include "resourceGroupBase.h"

namespace cyclonite::resources {
template<ManagedResourceConcept... Resources>
class ResourceGroup final : public ResourceGroupBase
{
public:
    template<typename R>
    constexpr static bool is_group_resource_type = metrix::type_list<Resources...>::template has_type<R>::value;

    explicit ResourceGroup(uint32_t id);

    template<CustomSourceConcept CustomSource>
    auto load(CustomSource&& customSource) -> std::future<void>;

    auto load(std::wstring_view location) -> std::future<void>;

    template<typename R, typename... Args>
    auto addResource(Args&&... args) -> core::ResourceUniqueRef
        requires(metrix::type_list<Resources...>::template has_type<R>::value);

    // void prepare

    // void unload

    // void getResource
private:
    static void makeOwn(core::ResourceBase* res);

    static void releaseOwnership(core::ResourceBase* res);

private:
    template<CustomSourceConcept CustomSource>
    static auto readSource(CustomSource* customSourcePtr) -> std::future<void>;

    uint32_t id_;
    core::ResourceManager<Resources...> resourcesLifetimeManager_;
};

template<ManagedResourceConcept... Resources>
template<CustomSourceConcept CustomSource>
/*static*/ auto ResourceGroup<Resources...>::readSource(CustomSource* customSourcePtr) -> std::future<void>
{
    assert(customSourcePtr != nullptr);
    return customSourcePtr->load();
}

template<ManagedResourceConcept... Resources>
ResourceGroup<Resources...>::ResourceGroup(uint32_t id)
  : ResourceGroupBase{}
  , id_{ id }
  , resourcesLifetimeManager_{}
{
}

template<ManagedResourceConcept... Resources>
template<typename R, typename... Args>
auto ResourceGroup<Resources...>::addResource(Args&&... args) -> core::ResourceUniqueRef
    requires(metrix::type_list<Resources...>::template has_type<R>::value)
{
    auto uniqueRes = resourcesLifetimeManager_.template allocResource<R>(std::forward<Args>(args)...);
    makeOwn(uniqueRes.resourceBase());

    return uniqueRes;
}

template<ManagedResourceConcept... Resources>
template<CustomSourceConcept CustomSource>
auto ResourceGroup<Resources...>::load(CustomSource&& customSource) -> std::future<void>
{
    static_assert(std::is_rvalue_reference_v<CustomSource>);

    auto source = std::make_unique<CustomSource>(std::move(customSource));
    source->setResourceGroup(this);
    return ResourceGroup<Resources...>::readSource<CustomSource>(&customSource);
}

template<ManagedResourceConcept... Resources>
auto ResourceGroup<Resources...>::load(std::wstring_view location) -> std::future<void>
{
    return load(DefaultResourceLoader{ location });
}

template<ManagedResourceConcept... Resources>
/*static */ void ResourceGroup<Resources...>::makeOwn(core::ResourceBase* res)
{
    assert(res != nullptr);
    res->retain();
}

template<ManagedResourceConcept... Resources>
/*static */ void ResourceGroup<Resources...>::releaseOwnership(core::ResourceBase* res)
{
    assert(res != nullptr);
    res->release();
}
}

#endif // CYCLONITE_RESOURCES_RESOURCE_GROUP_H
