//
// Created by anton on 10/4/26.
//

#include "geometryIndicesAllocation.h"
#include "geometryManager.h"

namespace cyclonite::gfx {
GeometryIndicesAllocation::~GeometryIndicesAllocation()
{
    reset();
}

void GeometryIndicesAllocation::reset()
{
    if (arena_ != nullptr) {
        arena_->free(firstIndex_, indexCount_);
        arena_ = nullptr;
    }
}
}
