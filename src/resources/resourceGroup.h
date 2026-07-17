//
// Created by anton on 3/9/26.
//

#ifndef CYCLONITE_RESOURCES_RESOURCE_GROUP_H
#define CYCLONITE_RESOURCES_RESOURCE_GROUP_H

#include "core/resourceManager.h"
#include <boost/uuid/uuid.hpp>
#include <metrix/type_list.h>

#include "defaultResourceLoader.h"
#include "resourceConcepts.h"

namespace cyclonite::resources {
template<ManagedResourceConcept... Resources>
class ResourceGroup final : public ResourceGroupBase
{
public:
    template<typename R>
    constexpr static bool is_group_resource_type = metrix::type_list<Resources...>::template has_type<R>::value;

    ResourceGroup(ResourceGroupManagerBase* groupManager, uint32_t id, core::ResourceSharedRef const& deviceRef);

    template<CustomSourceConcept CustomSource>
    auto load(CustomSource&& customSource) -> std::future<void>;

    auto load(std::wstring_view location) -> std::future<void>;

    auto prepare() -> std::future<void>;

    template<typename R, typename... Args>
    auto addResource(Args&&... args) -> core::ResourceUniqueRef
        requires(metrix::type_list<Resources...>::template has_type<R>::value);

    void releaseAll();

    void releaseResource(boost::uuids::uuid const& uuid);

    [[nodiscard]] auto isExists(boost::uuids::uuid const& uuid) const -> bool;

    [[nodiscard]] auto getResource(boost::uuids::uuid const& uuid) const -> core::ResourceSharedRef;

    [[nodiscard]] auto getResource(std::string_view name) const -> core::ResourceSharedRef;

private:
    static void makeOwn(core::ResourceBase* res);

    static void releaseOwnership(core::ResourceBase* res);

private:
    template<CustomSourceConcept CustomSource>
    static auto readSource(CustomSource* customSourcePtr) -> std::future<void>;

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
ResourceGroup<Resources...>::ResourceGroup(ResourceGroupManagerBase* groupManager,
                                           uint32_t id,
                                           core::ResourceSharedRef const& deviceRef)
  : ResourceGroupBase{ groupManager, id, deviceRef }
  , resourcesLifetimeManager_{}
{
}

template<ManagedResourceConcept... Resources>
template<typename R, typename... Args>
auto ResourceGroup<Resources...>::addResource(Args&&... args) -> core::ResourceUniqueRef
    requires(metrix::type_list<Resources...>::template has_type<R>::value)
{
    auto uniqueRes = resourcesLifetimeManager_.template allocResource<R>(this, std::forward<Args>(args)...);
    makeOwn(uniqueRes.resourceBase());

    return uniqueRes;
}

template<ManagedResourceConcept... Resources>
template<CustomSourceConcept CustomSource>
auto ResourceGroup<Resources...>::load(CustomSource&& customSource) -> std::future<void>
{
    auto source = std::make_unique<CustomSource>(std::move(customSource));
    source->setResourceGroup(*this);
    return ResourceGroup<Resources...>::readSource<CustomSource>(source.get());
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
auto prepare_if_resource_type_match(size_t typeId, core::ResourceSharedRef& ref, std::shared_future<void>& f) -> bool
{
    if (index == typeId) {
        auto& r = ref.as<ResType>();
        f = r.prepare();
        return true;
    }
    return false;
}
}

template<ManagedResourceConcept... Resources>
void ResourceGroup<Resources...>::releaseAll()
{
    using res_type_list_t = metrix::type_list<Resources...>;
    auto&& resList = resourcesLifetimeManager_.template resourceList<Resources...>();

    for (auto&& [ref, typeId] : resList) {
        auto result =
          ((internal::mark_to_remove_if_resource_type_match<Resources,
                                                            res_type_list_t::template get_type_index<Resources>::value>(
             typeId, ref)) ||
           ...);

        if (result) {
            auto& baseRes = ref.template as<core::ResourceBase>();
            releaseOwnership(&baseRes);
        }
    }
}

template<ManagedResourceConcept... Resources>
void ResourceGroup<Resources...>::releaseResource(boost::uuids::uuid const& uuid)
{
    using res_type_list_t = metrix::type_list<Resources...>;

    auto&& resList = resourcesLifetimeManager_.template resourceList<Resources...>();
    for (auto&& [ref, typeId] : resList) {
        auto result =
          ((internal::mark_to_remove_if_resource_type_match<Resources,
                                                            res_type_list_t::template get_type_index<Resources>::value>(
             typeId, ref, uuid)) ||
           ...);

        if (result) {
            auto& baseRes = ref.template as<core::ResourceBase>();
            releaseOwnership(&baseRes);
            break;
        }
    }
}

template<ManagedResourceConcept... Resources>
auto ResourceGroup<Resources...>::isExists(boost::uuids::uuid const& uuid) const -> bool
{
    using res_type_list_t = metrix::type_list<Resources...>;

    auto&& resList = resourcesLifetimeManager_.template resourceList<Resources...>();
    for (auto&& [ref, typeId] : resList) {
        auto result =
          ((internal::test_if_resource_type_match<Resources,
                                                  res_type_list_t::template get_type_index<Resources>::value>(
             typeId, ref, uuid)) ||
           ...);

        if (result) {
            return true;
        }
    }
    return false;
}

template<ManagedResourceConcept... Resources>
auto ResourceGroup<Resources...>::getResource(std::string_view name) const -> core::ResourceSharedRef
{
    using res_type_list_t = metrix::type_list<Resources...>;

    auto res = core::ResourceSharedRef{};

    auto&& resList = resourcesLifetimeManager_.template resourceList<Resources...>();
    for (auto&& [ref, typeId] : resList) {
        auto result =
          ((internal::test_if_resource_type_match<Resources,
                                                  res_type_list_t::template get_type_index<Resources>::value>(
             typeId, ref, name)) ||
           ...);

        if (result) {
            res = ref;
            break;
        }
    }

    return res;
}

template<ManagedResourceConcept... Resources>
auto ResourceGroup<Resources...>::getResource(boost::uuids::uuid const& uuid) const -> core::ResourceSharedRef
{
    using res_type_list_t = metrix::type_list<Resources...>;

    auto res = core::ResourceSharedRef{};

    auto&& resList = resourcesLifetimeManager_.template resourceList<Resources...>();
    for (auto&& [ref, typeId] : resList) {
        auto result =
          ((internal::test_if_resource_type_match<Resources,
                                                  res_type_list_t::template get_type_index<Resources>::value>(
             typeId, ref, uuid)) ||
           ...);

        if (result) {
            res = ref;
            break;
        }
    }

    return res;
}

template<ManagedResourceConcept... Resources>
auto ResourceGroup<Resources...>::prepare() -> std::future<void>
{
    using res_type_list_t = metrix::type_list<Resources...>;

    auto resCount = resourcesLifetimeManager_.template resourceCount<Resources...>();
    auto&& resList = resourcesLifetimeManager_.template resourceList<Resources...>();

    auto futures = std::vector<std::shared_future<void>>{};
    futures.reserve(resCount);

    for (auto&& [ref, typeId] : resList) {
        auto f = std::shared_future<void>{};
        auto result =
          ((internal::prepare_if_resource_type_match<Resources,
                                                     res_type_list_t::template get_type_index<Resources>::value>(
             typeId, ref, f)) ||
           ...);

        if (result) {
            futures.push_back(std::move(f));
        }
    }

    return multithreading::when_all(futures);
}
}

#endif // CYCLONITE_RESOURCES_RESOURCE_GROUP_H
