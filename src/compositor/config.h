//
// Created by bantdit on 1/28/19.
//

#ifndef CYCLONITE_CONFIG_H
#define CYCLONITE_CONFIG_H

#include <easy-mp/enum.h>
#include <easy-mp/type_list.h>
#include <enttx/componentStorage.h>
#include <enttx/enttx.h>

#include "components/animator.h"
#include "components/animatorStorage.h"
#include "components/camera.h"
#include "components/mesh.h"
#include "components/meshStorage.h"
#include "components/transform.h"
#include "components/transformStorage.h"
#include "systems/animationSystem.h"
#include "systems/cameraSystem.h"
#include "systems/meshSystem.h"
#include "systems/renderSystem.h"
#include "systems/transformSystem.h"
#include "systems/uniformSystem.h"
#include "systems/updateStages.h"
// #include "logicNode.h"
// #include "graphicsNode.h" // TODO::

namespace cyclonite::compositor {
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

using default_ecs_config_t = EcsConfig<
  easy_mp::type_list<components::Transform, components::Mesh, components::Camera, components::Animator>,
  easy_mp::type_list<components::TransformStorage<32, 1>,
                     components::MeshStorage<1024>,
                     enttx::ComponentStorage<1, 1, components::Camera>,
                     components::AnimatorStorage>,
  easy_mp::type_list<systems::AnimationSystem, systems::TransformSystem, systems::CameraSystem, systems::MeshSystem>,
  easy_mp::value_cast(UpdateStage::COUNT)>;

namespace config_traits {
template<typename T>
struct is_logic_node
{
private:
    using yes_t = uint8_t;
    using no_t = uint16_t;

    template<typename C>
    static constexpr auto test_field(std::bool_constant<C::is_logic_node_v>*) -> yes_t;

    template<typename C>
    static constexpr auto test_filed(...) -> no_t;

    static constexpr auto has_logic_field = sizeof(test_field<T>(0)) == sizeof(yes_t);

public:
    static constexpr auto value() -> bool
    {
        if constexpr (has_logic_field) {
            return T::is_logic_node_v;
        } else {
            return true;
        }
    }
};

template<typename T>
inline constexpr auto is_logic_node_v = is_logic_node<T>::value();

template<typename T>
struct is_surface_node
{
private:
    using yes_t = uint8_t;
    using no_t = uint16_t;

    template<typename C>
    static constexpr auto test_field(std::bool_constant<C::is_surface_node_v>*) -> yes_t;

    template<typename C>
    static constexpr auto test_filed(...) -> no_t;

    static constexpr auto has_surface_field = sizeof(test_field<T>(0)) == sizeof(yes_t);

public:
    static constexpr auto value() -> bool
    {
        if constexpr (has_surface_field) {
            return T::is_surface_node_v;
        } else {
            return false;
        }
    }
};

template<typename T>
struct get_pass_count
{
private:
    using yes_t = uint8_t;
    using no_t = uint16_t;

    template<typename C>
    static constexpr auto test_field(std::integral_constant<uint8_t, C::pass_count_v>*) -> yes_t;

    template<typename C>
    static constexpr auto test_filed(...) -> no_t;

    static constexpr auto has_pass_count_field = sizeof(test_field<T>(0)) == sizeof(yes_t);

public:
    static constexpr auto value() -> uint8_t
    {
        if constexpr (has_pass_count_field) {
            return T::pass_count_v;
        } else {
            return 0;
        }
    }
};

template<typename T>
struct get_ecs_config
{
private:
    using yes_t = uint8_t;
    using no_t = uint16_t;

    template<typename C>
    static constexpr auto test_field(typename C::ecs_config_t*) -> yes_t;

    template<typename C>
    static constexpr auto test_filed(...) -> no_t;

    static constexpr auto has_ecs_config = sizeof(test_field<T>(0)) == sizeof(yes_t);

public:
    using type = std::conditional_t<has_ecs_config, typename T::ecs_config_t, default_ecs_config_t>;
};

template<typename T>
using get_ecs_config_t = typename get_ecs_config<T>::type;
}

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

template<typename T>
concept NodeConfig = (config_traits::is_logic_node<T>::value() && !config_traits::is_surface_node<T>::value() &&
                      config_traits::get_pass_count<T>::value() == 0) ||
                     !config_traits::is_logic_node<T>::value();

namespace node_traits {
template<typename NodeType>
struct get_node_config;

template<template<typename> typename NodeType, NodeConfig Config>
struct get_node_config<NodeType<Config>>
{
    using type = Config;
};

template<typename NodeType>
using node_config_t = typename get_node_config<NodeType>::type;
}
}

#endif // CYCLONITE_CONFIG_H
