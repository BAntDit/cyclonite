//
// Created by anton on 8/14/25.
//

#ifndef CYCLONITE_CONFIGTRAITS_H
#define CYCLONITE_CONFIGTRAITS_H

#include "core/configTraitMacro.h"
#include <cstdint>

namespace cyclonite::gfx {
namespace internal {
template<typename T>
struct config_traits_declaration
{
    using yes_t = uint8_t;
    using no_t = uint16_t;

    // declare new parameters here, if necessary
    DECLARE_CONFIG_TRAIT(max_color_attachment_count, uint32_t, 8)
};
}

template<typename Config>
struct ConfigTraits
{
    // define new parameters here, if necessary
    DEFINE_CONFIG_TRAIT(max_color_attachment_count)
};
}

#endif // CYCLONITE_CONFIGTRAITS_H
