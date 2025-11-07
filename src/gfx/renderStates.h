//
// Created by anton on 11/6/25.
//

#ifndef CYCLONITE_GFX_RENDER_STATES_H
#define CYCLONITE_GFX_RENDER_STATES_H

#include "gfx/common.h"

namespace cyclonite::gfx {
struct StencilState
{
    StencilState() { setDefaults(); }

    void setDefaults()
    {
        failOp = StencilOp::KEEP;
        depthFailOp = StencilOp::KEEP;
        passOp = StencilOp::KEEP;
        compareOp = CompareOp::ALWAYS;
        compareMask = 0;
        writeMask = 0;
        reference = 0;
    }

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
    RasterizationState() { setDefaults(); }

    void setDefaults()
    {
        flags = RasterizationStateFlagBits{ RasterizationStateFlags::DEPTH_TEST_ENABLE,
                                            RasterizationStateFlags::DEPTH_WRITE_ENABLE,
                                            RasterizationStateFlags::RASTERIZER_DISCARD_ENABLE };

        depthComparison = CompareOp::ALWAYS;
        polygonMode = PolygonMode::FILL;
        cullMode = CullMode::NONE;
        frontFace = FrontFace::COUNTER_CLOCKWISE;
        frontStencilState = StencilState{};
        backStencilState = StencilState{};
        depthBiasConstantFactor = 0.f;
        depthBiasClamp = 0.f;
        depthBiasSlopeFactor = 0.f;
        lineWidth = 1.f;
    }

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