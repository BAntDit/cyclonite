//
// Created by anton on 3/8/26.
//

#ifndef CYCLONITE_RESOURCES_RESOURCE_LOADING_STATE_H
#define CYCLONITE_RESOURCES_RESOURCE_LOADING_STATE_H

#include <cstdint>

namespace cyclonite::resources {
enum class LoadingState : uint_fast8_t
{
    Undefined = 0,
    Initial = 1,
    Initializing = 2,
    Unloading = 3,
    Loading = 4,
    Loaded = 5,
    Preparing = 6,
    Ready = 7,
    RawPartUnloadedReady = 8,
    Reseting = 9,
    Corrupted = 10,
    Unloaded = 12
};
}

#endif // CYCLONITE_RESOURCES_RESOURCE_LOADING_STATE_H