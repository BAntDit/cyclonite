//
// Created by anton on 8/14/25.
//

#ifndef CYCLONITE_GFX_CONFIG_TRAITS_H
#define CYCLONITE_GFX_CONFIG_TRAITS_H

#include "core/configTraitMacro.h"
#include <cstddef>
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
    DECLARE_CONFIG_TRAIT(max_swapchain_length, uint8_t, 8)
    DECLARE_CONFIG_TRAIT(max_command_pool_count, size_t, 128)
};
}

template<typename Config>
struct ConfigTraits
{
    // define new parameters here, if necessary
    DEFINE_CONFIG_TRAIT(max_color_attachment_count)
    DEFINE_CONFIG_TRAIT(max_swapchain_length)
    DEFINE_CONFIG_TRAIT(max_command_pool_count)
};
}

#endif // CYCLONITE_GFX_CONFIG_TRAITS_H
