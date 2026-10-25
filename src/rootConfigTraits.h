//
// Created by anton on 4/5/26.
//

#ifndef CYCLONITE_ROOT_CONFIG_TRAITS_H
#define CYCLONITE_ROOT_CONFIG_TRAITS_H

#include "core/configTraitMacro.h"
#include "enttx/configTraits.h"
#include "geometry.h"
#include "material.h"
#include "scene.h"
#include "shader.h"
#include "systems/stages.h"
#include "systems/transferSystem.h"
#include "systems/transformSystem.h"
#include <metrix/type_list.h>

namespace cyclonite {
namespace internal {
struct DefaultEnttxConfig
{};

using default_system_list_t = metrix::type_list<systems::TransferSystem, systems::TransformSystem>;

using required_resource_list_t = metrix::type_list<Shader, Material, Geometry, Scene>;

template<typename T>
struct config_traits_declaration
{
    using yes_t = uint8_t;
    using no_t = uint16_t;

    DECLARE_CONFIG_TYPE_TRAIT(custom_resource_type_list, metrix::type_list<>)
    DECLARE_CONFIG_TYPE_TRAIT(system_update_stage_enum, systems::SystemUpdateStageList)
    DECLARE_CONFIG_TYPE_TRAIT(system_list, default_system_list_t)
    DECLARE_CONFIG_TYPE_TRAIT(transfer_system, systems::TransferSystem)
    DECLARE_CONFIG_TYPE_TRAIT(enttx_config, DefaultEnttxConfig)
};
}

template<typename Config>
struct ConfigTraits
{
    DEFINE_CONFIG_TYPE_TRAIT(custom_resource_type_list)
    DEFINE_CONFIG_TYPE_TRAIT(system_update_stage_enum)
    DEFINE_CONFIG_TYPE_TRAIT(system_list)
    DEFINE_CONFIG_TYPE_TRAIT(transfer_system)

    using enttx_config_t = enttx::ConfigTraits<typename internal::config_traits_declaration<Config>::enttx_config>;

    using resource_type_list_t = metrix::distinct<
      typename metrix::concat<custom_resource_type_list_t, internal::required_resource_list_t>::type>::type;
};
}

#endif // CYCLONITE_ROOT_CONFIG_TRAITS_H
