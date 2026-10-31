//
// Created by anton on 10/28/26.
//

#ifndef CYCLONITE_RESOURCES_RESOURCE_GROUP_H
#define CYCLONITE_RESOURCES_RESOURCE_GROUP_H

#include "core/resourceManager.h"
#include "resourceGroupBase.h"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <string_view>

namespace cyclonite::resources {
template<typename... ResourceType>
class ResourceGroup : public ResourceGroupBase
{
public:
    ResourceGroup(ResourceGroupManagerBase& groupManager, uint32_t id);

    template<typename R, typename... Args>
    auto addResource(std::string_view name, boost::uuids::uuid const& uuid, Args&&... args) -> core::ResourceSharedRef;

    [[nodiscard]] auto isExists(boost::uuids::uuid const& uuid) const -> bool;

    [[nodiscard]] auto isExists(std::string_view name) const -> bool;

    [[nodiscard]] auto getResource(boost::uuids::uuid const& uuid) const -> core::ResourceSharedRef;

    [[nodiscard]] auto getResource(std::string_view name) const -> core::ResourceSharedRef;

    void releaseResource(boost::uuids::uuid const& uuid);

    void releaseResource(std::string_view name);

    template<typename... ResourceFilter>
    class ResourceList
    {
        friend class ResourceGroup<ResourceType...>;

    public:
        class Iterator
        {
        public:
            using iterator_category = std::input_iterator_tag;
            using value_type = std::conditional_t<
              sizeof...(ResourceFilter) == 0,
              std::tuple<core::ResourceSharedRef, std::string_view, boost::uuids::uuid, std::variant<ResourceType*...>>,
              std::
                tuple<core::ResourceSharedRef, std::string_view, boost::uuids::uuid, std::variant<ResourceFilter*...>>>;

            using difference_type = uint32_t;
            using pointer = value_type*;
            using reference = value_type&;

            auto operator++() -> Iterator&;

            auto operator==(Iterator const& rhs) const -> bool { return it_ == rhs.it_; }
            auto operator!=(Iterator const& rhs) const -> bool { return it_ != rhs.it_; }

            auto operator*() const -> value_type;

        private:
            friend class ResourceList;

            Iterator(std::unordered_map<std::string_view, core::ResourceSharedRef>::iterator it,
                     std::unordered_map<std::string_view, core::ResourceSharedRef>::iterator end)
              : it_{ it }
              , end_{ end }
            {
                next();
            }

            void next();

            std::unordered_map<std::string_view, core::ResourceSharedRef>::iterator it_;
            std::unordered_map<std::string_view, core::ResourceSharedRef>::iterator end_;
        };

        auto begin() const -> Iterator {
            return Iterator{ group_->resourceNameToRef_.begin(), group_->resourceNameToRef_.end() };
        }

        auto end() const -> Iterator {
            return Iterator{ group_->resourceNameToRef_.end(), group_->resourceNameToRef_.end() };
        }

    private:
        ResourceList(ResourceGroup& group)
          : group_{ group }
        {
        }

        ResourceGroup* group_;
    };

private:
    static auto getResourceName(core::ResourceSharedRef const& ref) -> std::string_view;

    static auto getResourceUuid(core::ResourceSharedRef const& ref) -> boost::uuids::uuid const&;

    core::ResourceManager<ResourceType...> resources_;
    std::unordered_map<std::string_view, core::ResourceSharedRef> resourceNameToRef_;
    std::unordered_map<std::string, core::ResourceSharedRef> resourceUuidToRef_;
};

template<typename... ResourceType>
auto ResourceGroup<ResourceType...>::getResourceName(core::ResourceSharedRef const& ref) -> std::string_view
{
    auto name = std::string_view{};

    (void)((metrix::type_list<ResourceType...>::template get_type_index<ResourceType>::value == ref.typeIndex()
              ? (name = ref.as<ResourceType>().name(), true)
              : false) ||
           ...);

    return name;
}

template<typename... ResourceType>
auto ResourceGroup<ResourceType...>::getResourceUuid(core::ResourceSharedRef const& ref) -> boost::uuids::uuid const&
{
    auto uuid = boost::uuids::uuid{};

    (void)((metrix::type_list<ResourceType...>::template get_type_index<ResourceType>::value == ref.typeIndex()
              ? (uuid = ref.as<ResourceType>().uuid(), true)
              : false) ||
           ...);

    return uuid;
}

template<typename... ResourceType>
ResourceGroup<ResourceType...>::ResourceGroup(ResourceGroupManagerBase& groupManager, uint32_t id)
  : ResourceGroupBase{ &groupManager, id }
  , resources_{}
  , resourceNameToRef_{}
  , resourceUuidToRef_{}
{
}

template<typename... ResourceType>
template<typename R, typename... Args>
auto ResourceGroup<ResourceType...>::addResource(std::string_view name,
                                                 boost::uuids::uuid const& uuid,
                                                 Args&&... args) -> core::ResourceSharedRef
{
    auto ref = core::ResourceSharedRef{ resources_.template allocResource<R>(name, uuid, std::forward<Args>(args)...) };

    resourceNameToRef_.emplace(name, ref);

    auto us = boost::uuids::to_string(uuid);
    resourceUuidToRef_.emplace(us, ref);

    return ref;
}

template<typename... ResourceType>
auto ResourceGroup<ResourceType...>::isExists(boost::uuids::uuid const& uuid) const -> bool
{
    auto us = boost::uuids::to_string(uuid);
    return resourceUuidToRef_.contains(us);
}

template<typename... ResourceType>
auto ResourceGroup<ResourceType...>::isExists(std::string_view name) const -> bool
{
    return resourceNameToRef_.contains(name);
}

template<typename... ResourceType>
auto ResourceGroup<ResourceType...>::getResource(boost::uuids::uuid const& uuid) const -> core::ResourceSharedRef
{
    auto ref = core::ResourceSharedRef{};
    auto us = boost::uuids::to_string(uuid);

    if (resourceUuidToRef_.contains(us)) {
        ref = resourceUuidToRef_.at(us);
    }
    return ref;
}

template<typename... ResourceType>
auto ResourceGroup<ResourceType...>::getResource(std::string_view name) const -> core::ResourceSharedRef
{
    auto ref = core::ResourceSharedRef{};

    if (resourceNameToRef_.contains(name)) {
        ref = resourceNameToRef_.at(name);
    }
    return ref;
}

template<typename... ResourceType>
void ResourceGroup<ResourceType...>::releaseResource(boost::uuids::uuid const& uuid)
{
    auto us = boost::uuids::to_string(uuid);
    if (resourceUuidToRef_.contains(us)) {
        auto ref = resourceUuidToRef_.at(us);

        resourceUuidToRef_.erase(us);

        auto name = getResourceName(ref);
        assert(resourceNameToRef_.contains(name));

        resourceNameToRef_.erase(name);
    }
}

template<typename... ResourceType>
void ResourceGroup<ResourceType...>::releaseResource(std::string_view name)
{
    if (resourceNameToRef_.contains(name)) {
        auto ref = resourceNameToRef_.at(name);

        resourceNameToRef_.erase(name);

        auto uuid = getResourceUuid(ref);
        auto us = boost::uuids::to_string(uuid);
        assert(resourceUuidToRef_.contains(us));

        resourceUuidToRef_.erase(us);
    }
}

template<typename... ResourceType>
template<typename... ResourceFilter>
void ResourceGroup<ResourceType...>::ResourceList<ResourceFilter...>::Iterator::next()
{
    if constexpr (sizeof...(ResourceFilter) == 0) {
        return;
    }

    while (it_ != end_) {
        auto&& [_, ref] = *it_;
        auto matchFilter =
          ((metrix::type_list<ResourceType...>::template get_type_index<ResourceFilter>::value == ref.typeIndex()) ||
           ...);

        if (matchFilter)
            break;

        it_++;
    }
}

template<typename... ResourceType>
template<typename... ResourceFilter>
auto ResourceGroup<ResourceType...>::ResourceList<ResourceFilter...>::Iterator::operator++() -> Iterator&
{
    it_++;
    next();
    return *this;
}

template<typename... ResourceType>
template<typename... ResourceFilter>
auto ResourceGroup<ResourceType...>::ResourceList<ResourceFilter...>::Iterator::operator*() const -> value_type
{
    auto [name, ref] = *it_;
    auto uuid = ResourceGroup<ResourceType...>::getResourceUuid(ref);
    auto v = std::
      conditional_t<sizeof...(ResourceFilter) == 0, std::variant<ResourceType*...>, std::variant<ResourceFilter*...>>{};

    if constexpr (sizeof...(ResourceFilter) == 0) {
        (void)((metrix::type_list<ResourceType...>::template get_type_index<ResourceType>::value == ref.typeIndex()
                  ? (v = &ref.as<ResourceType>(), true)
                  : false) ||
               ...);
    } else {
        (void)((metrix::type_list<ResourceType...>::template get_type_index<ResourceFilter>::value == ref.typeIndex()
                  ? (v = &ref.as<ResourceFilter>(), true)
                  : false) ||
               ...);
    }

    return std::make_tuple(ref, name, uuid, v);
}
}

#endif // CYCLONITE_RESOURCES_RESOURCE_GROUP_H
