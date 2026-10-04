//
// Created by anton on 10/4/26.
//

#include "geometryIndicesAllocation.h"
#include "systems/geometryManagementSystem.h"

namespace cyclonite::gfx {
GeometryIndicesAllocation::~GeometryIndicesAllocation()
{
    if (arena_ != nullptr) {
        arena_->free(firstIndex_, indexCount_);
        arena_ = nullptr;
    }
}
}
