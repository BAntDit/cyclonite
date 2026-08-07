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

    template<typename C, typename R = void>
    using enable_if_component = std::enable_if_t<component_list_t::template has_type<C>::value, R>;

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
    auto getComponent(Entity entity) const -> enable_if_component<Component, Component const&>;

    template<typename Component>
    auto getComponent(Entity entity) -> enable_if_component<Component, Component&>;

private:
    template<typename Component>
    auto removeComponent(Entity entity) -> enable_if_component<Component>;

    using storage_tuple_t = typename meta_t::storage_tuple_t;
    using component_mask_t = typename meta_t::component_mask_t;

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
}

#endif // CYCLONITE_ENTITY_MANAGER_H