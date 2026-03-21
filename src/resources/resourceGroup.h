//
// Created by anton on 3/9/26.
//

#ifndef CYCLONITE_RESOURCES_RESOURCE_GROUP_H
#define CYCLONITE_RESOURCES_RESOURCE_GROUP_H

#include "core/resourceManager.h"
#include <concepts>
#include <future>
#include <metrix/type_list.h>
#include <metrix/type_traits.h>
#include <string>
#include <type_traits>

namespace cyclonite::resources {
class ResourceGroupBase
{
public:
    virtual ~ResourceGroupBase() = default;

    virtual auto load() -> std::future<void> = 0;
};

template<typename T>
concept ManagedResourceConcept =
  requires(T t) { requires std::is_move_constructible_v<T> || std::is_copy_constructible_v<T>; };

template<typename T>
concept CustomSourceConcept = requires(T t) {
    requires std::is_member_function_pointer_v<decltype(&T::setResourceGroup)> &&
               std::is_base_of_v<ResourceGroupBase,
                                 typename metrix::member_function_argument_type_list_t<
                                   decltype(&T::setResourceGroup)>::template get_type<0>::type>;

    { t.load() } -> std::same_as<std::future<void>>;
};

template<ManagedResourceConcept... Resources>
class ResourceGroup : public ResourceGroupBase
{
public:
    template<typename R>
    constexpr static bool is_group_resource_type = metrix::type_list<Resources...>::template has_type<R>::value;

    template<CustomSourceConcept CustomSource>
    ResourceGroup(uint32_t id, CustomSource&& customSource);

    ResourceGroup(uint32_t id, std::wstring_view sourceLocation);

    ~ResourceGroup() override = default;

    auto load() -> std::future<void> override;

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
    static auto readSource(void* customSourcePtr) -> std::future<void>;

    using load_f = std::future<void> (*)(void*);

    uint32_t id_;
    std::pair<void*, load_f> source_;
    core::ResourceManager<Resources...> resourcesLifetimeManager_;
};

template<ManagedResourceConcept... Resources>
template<CustomSourceConcept CustomSource>
/*static*/ auto ResourceGroup<Resources...>::readSource(void* customSourcePtr) -> std::future<void>
{
    assert(customSourcePtr != nullptr);
    return std::launder(reinterpret_cast<CustomSource*>(customSourcePtr))->load();
}

template<ManagedResourceConcept... Resources>
template<CustomSourceConcept CustomSource>
ResourceGroup<Resources...>::ResourceGroup(uint32_t id, CustomSource&& customSource)
  : ResourceGroupBase{}
  , id_{ id }
  , source_{ &customSource, &ResourceGroup<Resources...>::readSource }
  , resourcesLifetimeManager_{}
{
}

template<ManagedResourceConcept... Resources>
ResourceGroup<Resources...>::ResourceGroup(uint32_t id, std::wstring_view sourceLocation)
  : ResourceGroupBase{}
  , id_{ id } //, source - create default source reader
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
auto ResourceGroup<Resources...>::load() -> std::future<void>
{
    auto [loader, invoker] = source_;
    assert(loader != nullptr);
    assert(invoker != nullptr);

    return invoker(loader);
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
