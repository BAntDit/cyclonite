//
// Created by anton on 4/5/26.
//

#ifndef CYCLONITE_ROOT_CONFIG_TRAITS_H
#define CYCLONITE_ROOT_CONFIG_TRAITS_H

#include "core/configTraitMacro.h"
#include "systems/stages.h"
#include "systems/geometryManagementSystem.h"
#include <metrix/type_list.h>

namespace cyclonite {
namespace internal {
struct DefualtEnttxConfig
{};

using default_system_list_t = metrix::type_list<systems::GeometryManagementSystem>;

template<typename T>
struct config_traits_declaration
{
    using yes_t = uint8_t;
    using no_t = uint16_t;

    DECLARE_CONFIG_TYPE_TRAIT(custom_resource_type_list, metrix::type_list<>)
    DECLARE_CONFIG_TYPE_TRAIT(system_update_stage_enum, systems::SystemUpdateStageList)
    DECLARE_CONFIG_TYPE_TRAIT(system_list, default_system_list_t)
    DECLARE_CONFIG_TYPE_TRAIT(enttx_config, DefualtEnttxConfig)
};
}

template<typename Config>
struct ConfigTraits
{
    DEFINE_CONFIG_TYPE_TRAIT(custom_resource_type_list)
    DEFINE_CONFIG_TYPE_TRAIT(system_update_stage_enum)
    DEFINE_CONFIG_TYPE_TRAIT(system_list)
    DEFINE_CONFIG_TYPE_TRAIT(enttx_config)
};
}

#endif // CYCLONITE_ROOT_CONFIG_TRAITS_H
