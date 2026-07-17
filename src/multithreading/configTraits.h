//
// Created by anton on 9/13/25.
//

#ifndef CYCLONITE_MT_CONFIG_TRAITS_H
#define CYCLONITE_MT_CONFIG_TRAITS_H

#include "core/configTraitMacro.h"
#include <cstddef>
#include <cstdint>

namespace cyclonite::multithreading {
namespace internal {
template<typename T>
struct config_traits_declaration
{
    using yes_t = uint8_t;
    using no_t = uint16_t;

    // declare new parameters here, if necessary
    DECLARE_CONFIG_TRAIT(spmc_queue_max_size, size_t, 1024)
    DECLARE_CONFIG_TRAIT(mpsc_queue_max_size, size_t, 1024)
    DECLARE_CONFIG_TRAIT(strand_queue_max_size, size_t, 256)
};
}

template<typename Config>
struct ConfigTraits
{
    // define new parameters here, if necessary
    DEFINE_CONFIG_TRAIT(spmc_queue_max_size)
    DEFINE_CONFIG_TRAIT(mpsc_queue_max_size)
    DEFINE_CONFIG_TRAIT(strand_queue_max_size)
};
}

#endif // CYCLONITE_MT_CONFIG_TRAITS_H
