//
// Created by anton on 9/13/25.
//

#ifndef CYCLONITE_MT_CONFIG_H
#define CYCLONITE_MT_CONFIG_H

#include "configTraits.h"

namespace cyclonite::multithreading {
namespace internal {
struct config
{};
}

using config_t = multithreading::ConfigTraits<internal::config>;
}

#endif // CYCLONITE_MT_CONFIG_H
