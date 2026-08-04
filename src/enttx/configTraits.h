//
// Created by anton on 8/4/26.
//

#ifndef CYCLONITE_ENTTX_CONFIG_TRAITS_H
#define CYCLONITE_ENTTX_CONFIG_TRAITS_H

#include "core/configTraitMacro.h"
#include <cstdint>
#include <metrix/type_list.h>
#include <components/camera.h>

namespace cyclonite::enttx
{
namespace internal
{
template<typename T>
struct config_traits_declaration
{
    using yes_t = uint8_t;
    using no_t = uint16_t;

    DECLARE_CONFIG_TYPE_TRAIT(component_type_list, metrix::type_list<components::Camera>)
};

template<typename Config>
struct ConfigTraits
{
    DEFINE_CONFIG_TYPE_TRAIT(component_type_list)
};
}
}

#endif //CYCLONITE_ENTTX_CONFIG_TRAITS_H