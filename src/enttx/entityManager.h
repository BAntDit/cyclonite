//
// Created by anton on 8/4/26.
//

#ifndef CYCLONITE_ENTITY_MANAGER_H
#define CYCLONITE_ENTITY_MANAGER_H

#include "entity.h"
#include "enttx/configTraits.h"
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
                                         void>;
};

template<typename... Components>
struct component_list_traits<metrix::type_list<Components...>>
{
    using component_list_t = metrix::type_list<typename component_storage_pair<Components>::component_t...>;

    using storage_tuple_t = std::tuple<typename component_storage_pair<Components>::storage_t...>;
};
}

template<typename Config>
class EntityManager
{
private:
    template<typename EnttManagerConfig>
    struct Meta
    {
        using component_list_t =
          typename internal::component_list_traits<typename EnttManagerConfig::component_list_t>::storage_tuple_t;

        using storage_tuple_t =
          typename internal::component_list_traits<typename EnttManagerConfig::component_type_list_t>::storage_tuple_t;
    };

public:
    using config_t = enttx::internal::ConfigTraits<Config>;

    using meta_t = Meta<config_t>;

    expicit EntityManager(size_t initialCapacity = 10000);

private:
    using storage_tuple_t = typename meta_t::storage_tuple_t;

    std::vector<uint32_t> versions_;
    std::vector<uint32_t> freeIndices_;
    storage_tuple_t storage_;
};
}

#endif // CYCLONITE_ENTITY_MANAGER_H