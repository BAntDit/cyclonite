//
// Created by anton on 4/5/26.
//

#ifndef CYCLONITE_ROOT_CONFIG_TRAITS_H
#define CYCLONITE_ROOT_CONFIG_TRAITS_H

#include "core/configTraitMacro.h"
#include "shader.h"
#include <metrix/type_list.h>

namespace cyclonite {
namespace internal {
template<typename T>
struct config_traits_declaration
{
    using yes_t = uint8_t;
    using no_t = uint16_t;

    DECLARE_CONFIG_TYPE_TRAIT(custom_resource_type_list, metrix::type_list<>)
};
}

template<typename Config>
struct ConfigTraits
{
    DEFINE_CONFIG_TYPE_TRAIT(custom_resource_type_list)
};
}

#endif // CYCLONITE_ROOT_CONFIG_TRAITS_H
