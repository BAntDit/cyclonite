//
// Created by anton on 8/4/26.
//

#ifndef CYCLONITE_ENTITY_MANAGER_H
#define CYCLONITE_ENTITY_MANAGER_H

#include "componentStorage.h"
#include "entity.h"
#include "enttx/configTraits.h"
#include <bitset>
#include <metrix/containers.h>
#include <metrix/type_list.h>
#include <metrix/type_traits.h>
#include <tuple>
#include <vector>

namespace cyclonite::enttx {
namespace internal {
template<typename ComponentList>
struct component_list_traits;

template<typename Component>
struct component_storage_pair_decompose;

template<typename Component, typename Storage>
struct component_storage_pair_decompose<metrix::type_pair<Component, Storage>>
{
    using component_t = Component;
    using storage_t = Storage;
};

template<typename Component>
struct component_storage_pair
{
    using component_t = std::conditional_t<metrix::is_specialization_of_v<Component, metrix::type_pair>,
                                           typename component_storage_pair_decompose<Component>::component_t,
                                           Component>;

    using storage_t = std::conditional_t<metrix::is_specialization_of_v<Component, metrix::type_pair>,
                                         typename component_storage_pair_decompose<Component>::storage_t,
                                         ComponentStorage<32, 4, component_t>>;
};

template<typename... Components>
struct component_list_traits<metrix::type_list<Components...>>
{
    using component_list_t = metrix::type_list<typename component_storage_pair<Components>::component_t...>;

    using storage_tuple_t = std::tuple<typename component_storage_pair<Components>::storage_t...>;
};
} // internal

template<typename T>
concept EntityContainerConcept = requires(T) {
    typename T::value_type;

    requires metrix::is_contiguous_v<T>;

    requires std::is_same_v<typename T::value_type, Entity>;
};

template<typename Config>
class EntityManager
{
private:
    template<typename EnttManagerConfig>
    struct Meta
    {
        using component_list_t =
          typename internal::component_list_traits<typename EnttManagerConfig::component_list_t>::component_list_t;

        using storage_tuple_t =
          typename internal::component_list_traits<typename EnttManagerConfig::component_type_list_t>::storage_tuple_t;

        using component_mask_t = std::bitset<component_list_t::size>;
    };

public:
    using config_t = enttx::internal::ConfigTraits<Config>;

    using meta_t = Meta<config_t>;

    using component_list_t = typename config_t::component_list_t;
    using component_mask_t = typename config_t::component_mask_t;

    using storage_tuple_t = typename meta_t::storage_tuple_t;

    template<typename C, typename R = void>
    using enable_if_component = std::enable_if_t<component_list_t::template has_type<C>::value, R>;

    template<typename R, typename... Cs>
    using enable_if_components = std::enable_if_t<(... && component_list_t::template has_type<Cs>::value), R>;

    explicit EntityManager(size_t initialCapacity = 10000);

    [[nodiscard]] auto size() const -> size_t { return versions_.size() - freeIndices_.size(); }

    [[nodiscard]] auto capacity() const -> size_t { return versions_.size(); }

    [[nodiscard]] auto isValid(Entity entity) const -> bool
    {
        return entity.index() < versions_.size() && versions_[entity.index()] == entity.version();
    }

    [[nodiscard]] auto create() -> Entity;

    template<EntityContainerConcept Container>
    auto create(Container&& entities) -> Container&&;

    void destroy(Entity entity);

    template<typename... Components>
    void removeComponents(Entity entity);

    template<typename Component, typename... Args>
    auto assign(Entity entity, Args&&... args) -> enable_if_component<Component, Component&>;

    template<typename Component>
    [[nodiscard]] auto getComponent(Entity entity) const -> enable_if_component<Component, Component const&>;

    template<typename Component>
    [[nodiscard]] auto getComponent(Entity entity) -> enable_if_component<Component, Component&>;

    template<typename Component>
    [[nodiscard]] auto tryGetComponent(Entity entity) const -> enable_if_component<Component, Component const*>;

    template<typename Component>
    [[nodiscard]] auto tryGetComponent(Entity entity) -> enable_if_component<Component, Component*>;

    template<typename... Cs>
    [[nodiscard]] auto tryGetComponents(Entity entity) const -> enable_if_components<std::tuple<Cs const*...>, Cs...>;

    template<typename... Cs>
    [[nodiscard]] auto tryGetComponents(Entity entity) -> enable_if_components<std::tuple<Cs*...>, Cs...>;

    template<typename Component>
    [[nodiscard]] auto hasComponent(Entity entity) const -> enable_if_component<Component, bool>;

    template<typename... Cs>
    [[nodiscard]] auto hasComponents(Entity entity) const -> enable_if_components<std::bitset<sizeof...(Cs)>, Cs...>;

    template<bool isConst, typename... FilterComponents>
    class View
    {
    private:
        using entity_manager_t =
          typename std::conditional_t<isConst, EntityManager<Config> const&, EntityManager<Config>&>;

    public:
        using filter_component_list_t = metrix::type_list<FilterComponents...>;

        class Iterator
        {
        public:
            using iterator_category = std::input_iterator_tag;
            using value_type = std::tuple<Entity, FilterComponents&...>;
            using difference_type = uint32_t;
            using pointer = value_type*;
            using reference = value_type&;

            auto operator++() -> Iterator&;

            auto operator==(Iterator const& rhs) const -> bool { return cursor_ == rhs.cursor_; }
            auto operator!=(Iterator const& rhs) const -> bool { return cursor_ != rhs.cursor_; }

            auto operator*() const -> std::tuple<Entity, FilterComponents&...>;

        private:
            friend class View<isConst, FilterComponents...>;

            Iterator(entity_manager_t entityManager, component_mask_t filter, uint32_t cursor, uint32_t endIndex)
              : cursor_{ cursor }
              , endIndex_{ endIndex }
              , filter_{ filter }
              , entityManager_{ entityManager }
            {
            }

            void next();

            uint32_t cursor_;
            uint32_t endIndex_;
            component_mask_t filter_;
            entity_manager_t entityManager_;
        };

        [[nodiscard]] auto begin() const -> Iterator;

        [[nodiscard]] auto end() const -> Iterator;

    private:
        friend class EntityManager<Config>;

        explicit View(entity_manager_t entityManager);

        template<ComponentStorageConcept StorageType>
        auto getIteratorIndices(StorageType& storage, size_t entityCount, uint32_t& firstIndex, uint32_t& lastIndex)
          -> size_t;

        entity_manager_t entityManager_;
        component_mask_t filter_;
    };

    template<typename... FilterComponents>
    auto getView() const -> typename EntityManager<Config>::template View<true, FilterComponents...>;

    template<typename... FilterComponents>
    auto getView() -> typename EntityManager<Config>::template View<false, FilterComponents...>;

private:
    template<typename Component>
    auto removeComponent(Entity entity) -> enable_if_component<Component>;

    std::vector<uint32_t> versions_;
    std::vector<uint32_t> freeIndices_;
    std::vector<component_mask_t> masks_;
    storage_tuple_t storage_;
};

template<typename Config>
EntityManager<Config>::EntityManager(size_t initialCapacity /* = 10000*/)
  : versions_{}
  , freeIndices_{}
  , masks_{}
  , storage_{}
{
    versions_.reserve(initialCapacity);
    freeIndices_.reserve(initialCapacity);
    masks_.reserve(initialCapacity);
}

template<typename Config>
auto EntityManager<Config>::create() -> Entity
{
    auto index = uint32_t{ 0 };
    auto version = uint32_t{ 0 };

    if (freeIndices_.empty()) {
        index = versions_.size();
        version = 1;

        versions_.emplace_back(version);
        masks_.emplace_back();
    } else {
        index = freeIndices_.back();
        version = versions_[index];

        freeIndices_.pop_back();
    }

    return Entity{ index, version };
}

template<typename Config>
template<EntityContainerConcept Container>
auto EntityManager<Config>::create(Container&& entities) -> Container&&
{
    auto count = entities.size();
    auto counter = size_t{ 0 };

    auto index = uint32_t{ 0 };
    auto version = uint32_t{ 0 };

    while (counter < count && !freeIndices_.empty()) {
        index = freeIndices_.back();
        freeIndices_.pop_back();

        version = versions_[index];

        entities[counter++] = Entity{ index, version };
    }

    {
        auto left = count - counter;
        auto size = versions_.size();
        auto capacity = versions_.capacity();

        if ((size + left) > capacity) {
            capacity = size + left;

            versions_.reserve(capacity);
            freeIndices_.reserve(capacity);
            masks_.reserve(capacity);
        }
    }

    while (counter < count) {
        index = versions_.size();
        version = 1;

        versions_.emplace_back(version);
        masks_.emplace_back();

        entities[counter++] = Entity{ index, version };
    }

    return std::forward<Container>(entities);
};

template<typename Config>
void EntityManager<Config>::destroy(Entity entity)
{
    assert(isValid(entity));

    [this, entity]<typename... Cs>(metrix::type_list<Cs...>) -> void {
        removeComponents<Cs...>(entity);
    }(component_list_t{});

    versions_[entity.index()]++;

    freeIndices_.push_back(entity.index());
}

template<typename Config>
template<typename... Components>
void EntityManager<Config>::removeComponents(Entity entity)
{
    (removeComponent<Components>(entity), ...);
}

template<typename Config>
template<typename Component>
auto EntityManager<Config>::removeComponent(Entity entity) -> enable_if_component<Component>
{
    assert(isValid(entity));

    if (masks_[entity.index()].test(component_list_t::template get_type_index<Component>::value)) {
        masks_[entity.index()].reset(component_list_t::template get_type_index<Component>::value);

        std::get<component_list_t::template get_type_index<Component>::value>(storage_).destroy(entity.index());
    }
}

template<typename Config>
template<typename Component, typename... Args>
auto EntityManager<Config>::assign(Entity entity, Args&&... args) -> enable_if_component<Component, Component&>
{
    assert(isValid(entity));

    removeComponent<Component>(entity);

    masks_[entity.index()].set(component_list_t::template get_type_index<Component>::value);

    return std::get<component_list_t::template get_type_index<Component>::value>(storage_).create(
      entity.index(), std::forward<Args>(args)...);
}

template<typename Config>
template<typename Component>
auto EntityManager<Config>::getComponent(Entity entity) const -> enable_if_component<Component, Component const&>
{
    assert(isValid(entity));
    assert(masks_[entity.index()].test(component_list_t::template get_type_index<Component>::value));

    return std::get<component_list_t::template get_type_index<Component>::value>(storage_).get(entity.index());
}

template<typename Config>
template<typename Component>
auto EntityManager<Config>::getComponent(Entity entity) -> enable_if_component<Component, Component&>
{
    return const_cast<Component&>(std::as_const(*this).template getComponent<Component>(entity));
}

template<typename Config>
template<typename Component>
auto EntityManager<Config>::tryGetComponent(Entity entity) const -> enable_if_component<Component, Component const*>
{
    assert(isValid(entity));
    return masks_[entity.index()].test(component_list_t::template get_type_index<Component>::value)
             ? &getComponent<Component>(entity)
             : nullptr;
}

template<typename Config>
template<typename Component>
auto EntityManager<Config>::tryGetComponent(Entity entity) -> enable_if_component<Component, Component*>
{
    return const_cast<Component*>(std::as_const(*this).template tryGetComponent<Component>(entity));
}

template<typename Config>
template<typename... Cs>
auto EntityManager<Config>::tryGetComponents(Entity entity) const
  -> enable_if_components<std::tuple<Cs const*...>, Cs...>
{
    return std::tuple<Cs const*...>(tryGetComponent<Cs>(entity)...);
}

template<typename Config>
template<typename... Cs>
auto EntityManager<Config>::tryGetComponents(Entity entity) -> enable_if_components<std::tuple<Cs*...>, Cs...>
{
    return std::tuple<Cs*...>(tryGetComponent<Cs>(entity)...);
}

template<typename Config>
template<typename Component>
auto EntityManager<Config>::hasComponent(Entity entity) const -> enable_if_component<Component, bool>
{
    assert(isValid(entity));
    return masks_[entity.index()].test(component_list_t::template get_type_index<Component>::value);
}

template<typename Config>
template<typename... Cs>
auto EntityManager<Config>::hasComponents(Entity entity) const
  -> enable_if_components<std::bitset<sizeof...(Cs)>, Cs...>
{
    auto result = std::bitset<sizeof...(Cs)>{};

    (result.set(hasComponent<Cs>(entity)), ...);

    return result;
}

template<typename Config>
template<bool isConst, typename... FilterComponents>
auto EntityManager<Config>::View<isConst, FilterComponents...>::Iterator::operator++()
  -> EntityManager<Config>::View<isConst, FilterComponents...>::Iterator&
{
    cursor_++;
    next();
    return *this;
}

template<typename Config>
template<bool isConst, typename... FilterComponents>
auto EntityManager<Config>::View<isConst, FilterComponents...>::Iterator::operator*() const
  -> std::tuple<Entity, FilterComponents&...>
{
    auto entity = Entity{ cursor_, entityManager_.versions_[cursor_] };

    [[maybe_unused]] auto components = entityManager_.template tryGetComponents<FilterComponents...>(entity);

    return std::tie(entity, (*std::get<FilterComponents*>(components))...);
}

template<typename Config>
template<bool isConst, typename... FilterComponents>
void EntityManager<Config>::View<isConst, FilterComponents...>::Iterator::next()
{
    if constexpr (sizeof...(FilterComponents) != 0) {
        while (cursor_ < endIndex_ && (entityManager_.masks_[cursor_] & filter_) != filter_) {
            cursor_++;
        }
    }
    if constexpr (sizeof...(FilterComponents) != 0) {
        while (cursor_ < endIndex_ && entityManager_.masks_[cursor_].none()) {
            cursor_++;
        }
    }
}

template<typename Config>
template<bool isConst, typename... FilterComponents>
template<ComponentStorageConcept StorageType>
auto EntityManager<Config>::View<isConst, FilterComponents...>::getIteratorIndices(StorageType& storage,
                                                                                   size_t entityCount,
                                                                                   uint32_t& firstIndex,
                                                                                   uint32_t& lastIndex) -> size_t
{
    if (storage.size() > entityCount) {
        entityCount = storage.size();
        firstIndex = storage.getFirstEntityIndex();
        lastIndex = storage.getLastEntityIndex();
    }

    return entityCount;
}

template<typename Config>
template<bool isConst, typename... FilterComponents>
EntityManager<Config>::View<isConst, FilterComponents...>::View(entity_manager_t entityManager)
  : entityManager_{ entityManager }
  , filter_{}
{
    static_assert(std::is_same_v<typename metrix::inner_join<component_list_t, filter_component_list_t>::type,
                                 filter_component_list_t>);

    (filter_.set(component_list_t::template get_type_index<FilterComponents>::value), ...);
}

template<typename Config>
template<bool isConst, typename... FilterComponents>
auto EntityManager<Config>::View<isConst, FilterComponents...>::begin() const -> Iterator
{
    auto entityCount = size_t{ 0 };
    auto firstIndex = uint32_t{ 0 };
    auto lastIndex = uint32_t{ 0 };

    ((entityCount = getIteratorIndices(
        std::get<EntityManager<Config>::component_list_t::template get_type_index<FilterComponents>::value>(
          entityManager_.storage_),
        entityCount,
        firstIndex,
        lastIndex)),
     ...);

    auto iterator = Iterator{ entityManager_, filter_, firstIndex, lastIndex };

    iterator.next();

    return iterator;
}

template<typename Config>
template<bool isConst, typename... FilterComponents>
auto EntityManager<Config>::View<isConst, FilterComponents...>::end() const -> Iterator
{
    auto entityCount = size_t{ 0 };
    auto firstIndex = uint32_t{ 0 };
    auto lastIndex = uint32_t{ 0 };

    ((entityCount = getIteratorIndices(
        std::get<EntityManager<Config>::component_list_t::template get_type_index<FilterComponents>::value>(
          entityManager_.storage_),
        entityCount,
        firstIndex,
        lastIndex)),
     ...);

    return Iterator{ entityManager_, filter_, lastIndex, lastIndex };
}

template<typename Config>
template<typename... FilterComponents>
auto EntityManager<Config>::getView() const -> typename EntityManager<Config>::template View<true, FilterComponents...>
{
    return View<true, FilterComponents...>{ *this };
}

template<typename Config>
template<typename... FilterComponents>
auto EntityManager<Config>::getView() -> typename EntityManager<Config>::template View<false, FilterComponents...>
{
    return View<false, FilterComponents...>{ *this };
}
}

#endif // CYCLONITE_ENTITY_MANAGER_H