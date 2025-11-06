//
// Created by anton on 11/6/25.
//

#ifndef CYCLONITE_GFX_RENDER_STATES_H
#define CYCLONITE_GFX_RENDER_STATES_H

#include "gfx/common.h"

namespace cyclonite::gfx {
struct StencilState
{
    StencilOp failOp;
    StencilOp depthFailOp;
    StencilOp passOp;
    CompareOp compareOp;
    uint32_t compareMask;
    uint32_t writeMask;
    uint32_t reference;
};

struct RasterizationState
{
    RasterizationStateFlagBits flags;
    CompareOp depthComparison;
    PolygonMode polygonMode;
    CullMode cullMode;
    FrontFace frontFace;
    StencilState frontStencilState;
    StencilState backStencilState;
    real depthBiasConstantFactor;
    real depthBiasClamp;
    real depthBiasSlopeFactor;
    real lineWidth;
};
}

#endif // CYCLONITE_GFX_RENDER_STATES_H