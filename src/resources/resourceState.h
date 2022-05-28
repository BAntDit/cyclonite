//
// Created by anton on 5/25/22.
//

#ifndef CYCLONITE_RESOURCESTATE_H
#define CYCLONITE_RESOURCESTATE_H

#include <cstdint>

namespace cyclonite::resources {
enum class ResourceState : uint8_t
{
    LOADING = 0,
    LOADED = 1,
    UNLOADING = 2,
    UNLOADED = 3,
    COMPLETE = 4,
    MIN_VALUE = LOADING,
    MAX_VALUE = COMPLETE,
    COUNT = MAX_VALUE + 1
};
}
#endif // CYCLONITE_RESOURCESTATE_H
