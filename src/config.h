//
// Created by bantdit on 1/28/19.
//

#ifndef CYCLONITE_CONFIG_H
#define CYCLONITE_CONFIG_H

#include <easy-mp/type_list.h>
#include <enttx/enttx.h>

namespace cyclonite {
template<typename ComponentList, typename ComponentStorageList, typename SystemList, size_t updateStageCount>
struct EcsConfig;

template<typename... Components, typename... Storages, typename... Systems, size_t updateStageCount>
struct EcsConfig<easy_mp::type_list<Components...>,
                 easy_mp::type_list<Storages...>,
                 easy_mp::type_list<Systems...>,
                 updateStageCount>
{
    using component_list_t = easy_mp::type_list<Components...>;
    using storage_list_t = easy_mp::type_list<Storages...>;
    using system_list_t = easy_mp::type_list<Systems...>;

    using entity_manager_config_t = enttx::EntityManagerConfig<component_list_t, storage_list_t>;

    using entity_manager_t = enttx::EntityManager<entity_manager_config_t>;

    using system_manager_t =
      enttx::SystemManager<enttx::SystemManagerConfig<updateStageCount, entity_manager_config_t, system_list_t>>;
};

template<typename EcsConfig>
struct Config;

template<typename... Components, typename... Storages, typename... Systems, size_t updateStageCount>
struct Config<EcsConfig<easy_mp::type_list<Components...>,
                        easy_mp::type_list<Storages...>,
                        easy_mp::type_list<Systems...>,
                        updateStageCount>>
{
    using ecs_config_t = EcsConfig<easy_mp::type_list<Components...>,
                                   easy_mp::type_list<Storages...>,
                                   easy_mp::type_list<Systems...>,
                                   updateStageCount>;
};
}

#endif // CYCLONITE_CONFIG_H
