//
// Created by anton on 8/14/25.
//

#ifndef CYCLONITE_GFX_CONFIG_H
#define CYCLONITE_GFX_CONFIG_H

#include "configTraits.h"

namespace cyclonite::gfx {
struct compile_time_config
{};

using config_t = ConfigTraits<compile_time_config>;
}

#endif // CYCLONITE_GFX_CONFIG_H
