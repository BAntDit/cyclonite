//
// Created by bantdit on 1/28/19.
//

#ifndef CYCLONITE_CONFIG_H
#define CYCLONITE_CONFIG_H

#include <enttx/componentStorage.h>
#include <enttx/enttx.h>
#include <metrix/enum.h>
#include <metrix/type_list.h>

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
using systems_config_t = enttx::SystemManagerConfig<updateStageCount, metrix::type_list<System...>>;

namespace internal {
template<typename T, template<typename, typename> typename Spec>
struct is_component_config_specialization : std::false_type
{};

template<typename... C, typename... S, template<typename, typename> typename Spec>
struct is_component_config_specialization<Spec<metrix::type_list<C...>, metrix::type_list<S...>>, Spec> : std::true_type
{};

// enttx::EntityManagerConfig instead of component_config_t according to paragraph 14.5.7/2 of the C++ Standard
// When a template-id refers to the specialization of an alias template, it is equivalent to the associated type
// obtained by substitution of its template-arguments for the template-parameters in the type-id of the alias template.
// [ Note: An alias template name is never deduced.—end note ]
template<typename T>
inline constexpr auto is_component_config_specialization_v =
  is_component_config_specialization<T, enttx::EntityManagerConfig>::value;

template<typename T, template<size_t, typename> typename Spec>
struct is_systems_config_specialization : std::false_type
{};

template<size_t stageCount, typename... Systems, template<size_t, typename> typename Spec>
struct is_systems_config_specialization<Spec<stageCount, metrix::type_list<Systems...>>, Spec> : std::true_type
{};

// enttx::SystemManagerConfig instead of systems_config_t according to paragraph 14.5.7/2 of the C++ Standard
// When a template-id refers to the specialization of an alias template, it is equivalent to the associated type
// obtained by substitution of its template-arguments for the template-parameters in the type-id of the alias template.
// [ Note: An alias template name is never deduced.—end note ]
template<typename T>
inline constexpr auto is_systems_config_specialization_v =
  is_systems_config_specialization<T, enttx::SystemManagerConfig>::value;
}

template<typename T>
concept ComponentConfig = internal::is_component_config_specialization_v<T>;

template<typename T>
concept SystemsConfig = internal::is_systems_config_specialization_v<T>;

using default_component_config_t = component_config_t<
  metrix::type_list<components::Transform, components::Mesh, components::Camera, components::Animator>,
  metrix::type_list<components::TransformStorage<32, 1>,
                    components::MeshStorage<1024>,
                    enttx::ComponentStorage<1, 1, components::Camera>,
                    components::AnimatorStorage>>;
// test:
static_assert(internal::is_component_config_specialization_v<default_component_config_t>);

using default_systems_config_t = systems_config_t<
  metrix::value_cast(systems::UpdateStage::COUNT),
  metrix::type_list<systems::AnimationSystem, systems::TransformSystem, systems::CameraSystem, systems::MeshSystem>>;

// test:
static_assert(internal::is_systems_config_specialization_v<default_systems_config_t>);

namespace config_traits {
template<typename T>
struct is_logic_node
{
private:
    using yes_t = uint8_t;
    using no_t = uint16_t;

    template<typename C>
    static constexpr auto test_field(decltype(&C::is_logic_node_v)) -> yes_t;

    template<typename C>
    static constexpr auto test_field(...) -> no_t;

    static constexpr auto has_logic_field = sizeof(test_field<T>(0)) == sizeof(yes_t);

public:
    static constexpr auto value() -> bool
    {
        if constexpr (has_logic_field) {
            if constexpr (std::is_same_v<std::decay_t<decltype(T::is_logic_node_v)>, bool>)
                return T::is_logic_node_v;
            else
                return false;
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
struct get_max_wait_semaphore_count
{
private:
    using yes_t = uint8_t;
    using no_t = uint16_t;

    template<typename C>
    static constexpr auto test_field(decltype(&C::max_wait_semaphore_count_v)) -> yes_t;

    template<typename C>
    static constexpr auto test_field(...) -> no_t;

    static constexpr auto has_max_wait_semaphore_count_field = sizeof(test_field<T>(0)) == sizeof(yes_t);

public:
    static constexpr auto value() -> size_t
    {
        if constexpr (is_graphics_node_v<T>) {
            if constexpr (has_max_wait_semaphore_count_field) {
                static_assert(std::is_nothrow_convertible_v<decltype(T::max_wait_semaphore_count_v), size_t>);
                return static_cast<size_t>(T::max_wait_semaphore_count_v);
            } else {
                return size_t{ 16 };
            }
        } else {
            return 0;
        }
    }
};

template<typename T>
inline constexpr auto max_wait_semaphore_count_v = get_max_wait_semaphore_count<T>::value();

template<typename T>
struct is_surface_node
{
private:
    using yes_t = uint8_t;
    using no_t = uint16_t;

    template<typename C>
    static constexpr auto test_field(decltype(&C::is_surface_node_v)) -> yes_t;

    template<typename C>
    static constexpr auto test_field(...) -> no_t;

    static constexpr auto has_surface_field = sizeof(test_field<T>(0)) == sizeof(yes_t);

public:
    static constexpr auto value() -> bool
    {
        if constexpr (has_surface_field) {
            if constexpr (std::is_same_v<std::decay_t<decltype(T::is_surface_node_v)>, bool>)
                return T::is_surface_node_v;
            else
                return false;
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
    static constexpr auto test_field(decltype(&C::pass_count_v)) -> yes_t;

    template<typename C>
    static constexpr auto test_field(...) -> no_t;

    static constexpr auto has_pass_count_field = sizeof(test_field<T>(0)) == sizeof(yes_t);

public:
    static constexpr auto value() -> uint8_t
    {
        if constexpr (has_pass_count_field) {
            static_assert(std::is_nothrow_convertible_v<decltype(T::pass_count_v), uint8_t>);
            return static_cast<uint8_t>(T::pass_count_v);
        } else {
            if constexpr (is_graphics_node_v<T>) {
                return 1;
            } else {
                return 0;
            }
        }
    }
};

template<typename T>
inline constexpr auto pass_count_v = get_pass_count<T>::value();
}

template<ComponentConfig CompConfig, SystemsConfig SysConfig>
struct Config
{
    using component_config_t = CompConfig;
    using systems_config_t = SysConfig;
};

namespace node_internal {
template<typename T, template<typename, typename> typename Spec>
struct is_node_config_specialization : std::false_type
{};

template<ComponentConfig CompConfig, SystemsConfig SysConfig, template<typename, typename> typename Spec>
struct is_node_config_specialization<Spec<CompConfig, SysConfig>, Spec> : std::true_type
{};

template<typename T>
inline constexpr auto is_node_config_specialization_v = is_node_config_specialization<T, Config>::value;
}

template<typename T>
concept NodeConfig = (node_internal::is_node_config_specialization_v<T> ||
                      std::is_base_of_v<Config<typename T::component_config_t, typename T::systems_config_t>, T>) &&
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
