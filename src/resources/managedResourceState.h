//
// Created by anton on 3/8/26.
//

#ifndef CYCLONITE_RESOURCES_MANAGED_RESOURCE_STATE_H
#define CYCLONITE_RESOURCES_MANAGED_RESOURCE_STATE_H

#include <cstdint>

namespace cyclonite::resources {
enum class ManagedResourceState : uint_fast8_t
{
    Initial = 1,
    Loading = 2,
    Loaded = 3,
    Preparing = 4,
    Ready = 5,
    GoingToBeRemoved = 6
};
}

#endif // CYCLONITE_RESOURCES_MANAGED_RESOURCE_STATE_H