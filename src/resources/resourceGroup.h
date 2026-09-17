//
// Created by anton on 3/9/26.
//

#ifndef CYCLONITE_RESOURCES_RESOURCE_GROUP_H
#define CYCLONITE_RESOURCES_RESOURCE_GROUP_H

#include "core/resourceManager.h"
#include <boost/uuid/uuid.hpp>
#include <metrix/type_list.h>

#include "defaultResourceLoader.h"
#include "material.h"
#include "resourceConcepts.h"
#include "rootConfigTraits.h"

namespace cyclonite {
template<typename Config>
class Root;
}

namespace cyclonite::resources {
template<typename Config>
class ResourceGroup : public ResourceGroupBase
{
public:
    using config_t = cyclonite::ConfigTraits<Config>;
    using default_resource_list_t = metrix::type_list<Shader, Material>;
    using resource_type_list_t = metrix::distinct<
      typename metrix::concat<typename config_t::custom_resource_type_list_t, default_resource_list_t>::type>::type;

    template<ManagedResourceConcept R>
    constexpr static bool is_group_resource_type = resource_type_list_t::template has_type<R>::value;

    ResourceGroup(Root<Config>& root, ResourceGroupManagerBase* groupManager, uint32_t id);

    template<CustomSourceConcept CustomSource>
    auto load(CustomSource&& customSource) -> std::future<void>;

    auto load(std::wstring_view location) -> std::future<void>;

    auto prepare(core::ResourceSharedRef const& deviceRef) -> std::future<void>;

    template<typename R, typename... Args>
    auto addResource(Args&&... args) -> core::ResourceUniqueRef
        requires(resource_type_list_t::template has_type<R>::value);

    void releaseAll();

    void releaseResource(boost::uuids::uuid const& uuid);

    [[nodiscard]] auto isExists(boost::uuids::uuid const& uuid) const -> bool;

    [[nodiscard]] auto getResource(boost::uuids::uuid const& uuid) const -> core::ResourceSharedRef;

    [[nodiscard]] auto getResource(std::string_view name) const -> core::ResourceSharedRef;

private:
    template<typename... Resource>
    auto prepareInternal(core::ResourceSharedRef const& deviceRef, metrix::type_list<Resource...>) -> std::future<void>;

    template<typename... Resource>
    void releaseAllInternal(metrix::type_list<Resource...>);

    template<typename... Resource>
    void releaseResourceInternal(boost::uuids::uuid const& uuid, metrix::type_list<Resource...>);

    template<typename... Resource>
    [[nodiscard]] auto isExistsInternal(boost::uuids::uuid const& uuid, metrix::type_list<Resource...>) const -> bool;

    template<typename... Resource>
    [[nodiscard]] auto getResourceInternal(boost::uuids::uuid const& uuid,
                                           metrix::type_list<Resource...>) const -> core::ResourceSharedRef;

    template<typename... Resource>
    [[nodiscard]] auto getResourceInternal(std::string_view name,
                                           metrix::type_list<Resource...>) const -> core::ResourceSharedRef;

    static void makeOwn(core::ResourceBase& res);

    static void releaseOwnership(core::ResourceBase& res);

    template<CustomSourceConcept CustomSource>
    static auto readSource(CustomSource* customSourcePtr) -> std::future<void>;

private:
    template<typename ResourceList>
    struct resources_meta;

    template<ManagedResourceConcept... Resource>
    struct resources_meta<metrix::type_list<Resource...>>
    {
        using resource_manager_type_t = core::ResourceManager<Resource...>;
    };
    using resource_manager_t = typename resources_meta<resource_type_list_t>::resource_manager_type_t;

    resource_manager_t resourcesLifetimeManager_;
    Root<Config>* root_;
};

template<typename Config>
template<CustomSourceConcept CustomSource>
/*static*/ auto ResourceGroup<Config>::readSource(CustomSource* customSourcePtr) -> std::future<void>
{
    assert(customSourcePtr != nullptr);
    return customSourcePtr->load();
}

template<typename Config>
ResourceGroup<Config>::ResourceGroup(Root<Config>& root, ResourceGroupManagerBase* groupManager, uint32_t id)
  : ResourceGroupBase{ groupManager, id }
  , resourcesLifetimeManager_{}
  , root_{ &root }
{
}

template<typename Config>
template<CustomSourceConcept CustomSource>
auto ResourceGroup<Config>::load(CustomSource&& customSource) -> std::future<void>
{
    auto source = std::make_unique<CustomSource>(std::move(customSource));
    source->setResourceGroup(*this);
    return ResourceGroup<Config>::readSource<CustomSource>(source.get());
}

template<typename Config>
auto ResourceGroup<Config>::load(std::wstring_view location) -> std::future<void>
{
    return load(DefaultResourceLoader{ location });
}

namespace internal {
template<typename ResType, size_t index>
auto test_if_resource_type_match(size_t typeId,
                                 core::ResourceSharedRef const& ref,
                                 boost::uuids::uuid const& uuid) -> bool
{
    if (index == typeId) {
        auto const& r = ref.as<ResType>();
        return r.uuid() == uuid;
    }
    return false;
}

template<typename ResType, size_t index>
auto test_if_resource_type_match(size_t typeId, core::ResourceSharedRef const& ref, std::string_view name) -> bool
{
    if (index == typeId) {
        auto const& r = ref.as<ResType>();
        return r.name() == name;
    }
    return false;
}

template<typename ResType, size_t index>
auto mark_to_remove_if_resource_type_match(size_t typeId,
                                           core::ResourceSharedRef& ref,
                                           boost::uuids::uuid const& uuid) -> bool
{
    if (index == typeId) {
        auto& r = ref.as<ResType>();
        if (r.uuid() == uuid) {
            r.markToRemove();
            return true;
        }
    }
    return false;
}

template<typename ResType, size_t index>
auto mark_to_remove_if_resource_type_match(size_t typeId, core::ResourceSharedRef& ref) -> bool
{
    if (index == typeId) {
        auto& r = ref.as<ResType>();
        r.markToRemove();
        return true;
    }
    return false;
}

template<typename ResType, size_t index>
auto prepare_if_resource_type_match(size_t typeId,
                                    core::ResourceSharedRef const& deviceRef,
                                    core::ResourceSharedRef& ref,
                                    std::shared_future<void>& f) -> bool
{
    if (index == typeId) {
        auto& r = ref.as<ResType>();
        f = r.prepare(deviceRef);
        return true;
    }
    return false;
}
}

template<typename Config>
template<typename... Resource>
auto ResourceGroup<Config>::prepareInternal(core::ResourceSharedRef const& deviceRef,
                                            metrix::type_list<Resource...>) -> std::future<void>
{
    auto resCount = resourcesLifetimeManager_.template resourceCount<Resource...>();
    auto&& resList = resourcesLifetimeManager_.template resourceList<Resource...>();

    auto futures = std::vector<std::shared_future<void>>{};
    futures.reserve(resCount);

    for (auto&& [ref, typeId] : resList) {
        auto f = std::shared_future<void>{};
        auto result =
          ((internal::prepare_if_resource_type_match<Resource,
                                                     resource_type_list_t::template get_type_index<Resource>::value>(
             typeId, deviceRef, ref, f)) ||
           ...);
        if (result) {
            futures.push_back(std::move(f));
        }
    }

    return multithreading::when_all(futures);
}

template<typename Config>
auto ResourceGroup<Config>::prepare(core::ResourceSharedRef const& deviceRef) -> std::future<void>
{
    return prepareInternal(deviceRef, resource_type_list_t{});
}

template<typename Config>
template<typename R, typename... Args>
auto ResourceGroup<Config>::addResource(Args&&... args) -> core::ResourceUniqueRef
    requires(resource_type_list_t::template has_type<R>::value)
{
    auto uniqueRes = resourcesLifetimeManager_.template allocResource<R>(this, std::forward<Args>(args)...);
    assert(uniqueRes.resourceBase() != nullptr);
    makeOwn(*uniqueRes.resourceBase());

    return uniqueRes;
}

template<typename Config>
/*static */ void ResourceGroup<Config>::makeOwn(core::ResourceBase& res)
{
    res.retain();
}

template<typename Config>
/*static */ void ResourceGroup<Config>::releaseOwnership(core::ResourceBase& res)
{
    res.release();
}

template<typename Config>
template<typename... Resource>
void ResourceGroup<Config>::releaseAllInternal(metrix::type_list<Resource...>)
{
    auto&& resList = resourcesLifetimeManager_.template resourceList<Resource...>();

    for (auto&& [ref, typeId] : resList) {
        auto result = ((internal::mark_to_remove_if_resource_type_match<
                         Resource,
                         resource_type_list_t::template get_type_index<Resource>::value>(typeId, ref)) ||
                       ...);

        if (result) {
            auto& baseRes = ref.template as<core::ResourceBase>();
            releaseOwnership(baseRes);
        }
    }
}

template<typename Config>
void ResourceGroup<Config>::releaseAll()
{
    return releaseAllInternal(resource_type_list_t{});
}

template<typename Config>
template<typename... Resource>
void ResourceGroup<Config>::releaseResourceInternal(boost::uuids::uuid const& uuid, metrix::type_list<Resource...>)
{
    auto&& resList = resourcesLifetimeManager_.template resourceList<Resource...>();
    for (auto&& [ref, typeId] : resList) {
        auto result = ((internal::mark_to_remove_if_resource_type_match<
                         Resource,
                         resource_type_list_t::template get_type_index<Resource>::value>(typeId, ref, uuid)) ||
                       ...);

        if (result) {
            auto& baseRes = ref.template as<core::ResourceBase>();
            releaseOwnership(baseRes);
            break;
        }
    }
}

template<typename Config>
void ResourceGroup<Config>::releaseResource(boost::uuids::uuid const& uuid)
{
    releaseAllInternal(uuid, resource_type_list_t{});
}

template<typename Config>
template<typename... Resource>
auto ResourceGroup<Config>::isExistsInternal(boost::uuids::uuid const& uuid,
                                             metrix::type_list<Resource...>) const -> bool
{
    auto&& resList = resourcesLifetimeManager_.template resourceList<Resource...>();
    for (auto&& [ref, typeId] : resList) {
        auto result =
          ((internal::test_if_resource_type_match<Resource,
                                                  resource_type_list_t::template get_type_index<Resource>::value>(
             typeId, ref, uuid)) ||
           ...);

        if (result) {
            return true;
        }
    }
    return false;
}

template<typename Config>
auto ResourceGroup<Config>::isExists(boost::uuids::uuid const& uuid) const -> bool
{
    return isExistsInternal(uuid, resource_type_list_t{});
}

template<typename Config>
template<typename... Resource>
auto ResourceGroup<Config>::getResourceInternal(std::string_view name,
                                                metrix::type_list<Resource...>) const -> core::ResourceSharedRef
{
    auto res = core::ResourceSharedRef{};
    auto&& resList = resourcesLifetimeManager_.template resourceList<Resource...>();
    for (auto&& [ref, typeId] : resList) {
        auto result =
          ((internal::test_if_resource_type_match<Resource,
                                                  resource_type_list_t::template get_type_index<Resource>::value>(
             typeId, ref, name)) ||
           ...);

        if (result) {
            res = ref;
            break;
        }
    }

    return res;
}

template<typename Config>
auto ResourceGroup<Config>::getResource(std::string_view name) const -> core::ResourceSharedRef
{
    return getResourceInternal(name, resource_type_list_t{});
}

template<typename Config>
template<typename... Resource>
auto ResourceGroup<Config>::getResourceInternal(boost::uuids::uuid const& uuid,
                                                metrix::type_list<Resource...>) const -> core::ResourceSharedRef
{
    auto res = core::ResourceSharedRef{};
    auto&& resList = resourcesLifetimeManager_.template resourceList<Resource...>();
    for (auto&& [ref, typeId] : resList) {
        auto result =
          ((internal::test_if_resource_type_match<Resource,
                                                  resource_type_list_t::template get_type_index<Resource>::value>(
             typeId, ref, uuid)) ||
           ...);

        if (result) {
            res = ref;
            break;
        }
    }

    return res;
}

template<typename Config>
auto ResourceGroup<Config>::getResource(boost::uuids::uuid const& uuid) const -> core::ResourceSharedRef
{
    return getResourceInternal(uuid, resource_type_list_t{});
}
}

#endif // CYCLONITE_RESOURCES_RESOURCE_GROUP_H
