//
// Created by anton on 5/25/22.
//

#ifndef CYCLONITE_RESOURCEDEPENDENCY_H
#define CYCLONITE_RESOURCEDEPENDENCY_H

#include "resourceState.h"

namespace cyclonite::resources {
struct ResourceDependency
{
    uint64_t id;
    ResourceState resourceState;
};
}

#endif // CYCLONITE_RESOURCEDEPENDENCY_H
