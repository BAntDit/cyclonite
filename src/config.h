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

namespace cyclonite {
template<typename ComponentList, typename StorageList>
using component_config_t = enttx::EntityManagerConfig<ComponentList, StorageList>;

template<size_t updateStageCount, typename... System>
using systems_config_t = enttx::SystemManagerConfig<updateStageCount, type_list<System...>>;

namespace internal {
template<typename T, template<typename, typename> typename Spec>
struct is_component_config_specialization : std::false_type
{};

template<typename... C, typename... S, template<typename, typename> typename Spec>
struct is_component_config_specialization<Spec<type_list<C...>, type_list<S...>>, Spec> : std::true_type
{};

template<typename T, template<typename, typename> typename Spec>
inline constexpr auto is_component_config_specialization_v = is_component_config_specialization<T, Spec>::value;

template<typename T, template<size_t, typename> typename Spec>
struct is_systems_config_specialization : std::false_type
{};

template<size_t stageCount, typename... Systems, template<size_t, typename> typename Spec>
struct is_systems_config_specialization<Spec<stageCount, type_list<Systems...>>, Spec> : std::true_type
{};

template<typename T, template<size_t, typename> typename Spec>
inline constexpr auto is_systems_config_specialization_v = is_systems_config_specialization<T, Spec>::value;
}

template<typename T>
concept ComponentConfig = internal::is_component_config_specialization_v<T, component_config_t>;

template<typename T>
concept SystemsConfig = internal::is_systems_config_specialization_v<T, systems_config_t>;

using default_component_config_t =
  component_config_t<type_list<components::Transform, components::Mesh, components::Camera, components::Animator>,
                     type_list<components::TransformStorage<32, 1>,
                               components::MeshStorage<1024>,
                               enttx::ComponentStorage<1, 1, components::Camera>,
                               components::AnimatorStorage>>;

using default_systems_config_t = systems_config_t<
  value_cast(UpdateStage::COUNT),
  type_list<systems::AnimationSystem, systems::TransformSystem, systems::CameraSystem, systems::MeshSystem>>;

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
            return false;
        }
    }
};

template<typename T>
inline constexpr auto is_logic_node_v = is_logic_node<T>::value();

template<typename T>
inline constexpr auto is_graphics_node_v = !is_logic_node<T>::value();

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
inline constexpr auto is_surface_node_v = is_surface_node<T>::value();

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
inline constexpr auto pass_count_v = get_pass_count<T>::value();
}

template<ComponentConfig CompConfig, SystemsConfig SysConfig>
struct Config;

template<typename... Components, typename... Storages, typename... Systems, size_t updateStageCount>
struct Config<component_config_t<type_list<Components...>, type_list<Storages...>>,
              systems_config_t<updateStageCount, type_list<Systems...>>>
{
    using component_config_t = component_config_t<type_list<Components...>, type_list<Storages...>>;
    using systems_config_t = systems_config_t<updateStageCount, type_list<Systems...>>;
};

namespace node_internal {
template<typename T, template<typename, typename> typename Spec>
struct is_node_config_specialization : std::false_type
{};

template<ComponentConfig CompConfig, SystemsConfig SysConfig, template<typename, typename> typename Spec>
struct is_node_config_specialization<Spec<CompConfig, SysConfig>, Spec> : std::true_type
{};

template<typename T, template<typename, typename> typename Spec>
inline constexpr auto is_node_config_specialization_v = is_node_config_specialization<T, Spec>::value;
}

template<typename T>
concept NodeConfig = node_internal::is_node_config_specialization_v<T, Config> &&
                     ((config_traits::is_logic_node<T>::value() && !config_traits::is_surface_node<T>::value() &&
                       config_traits::get_pass_count<T>::value() == 0) ||
                      !config_traits::is_logic_node<T>::value());

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
