//
// Created by anton on 2/5/23.
//

#ifndef CYCLONITE_MATERIAL_H
#define CYCLONITE_MATERIAL_H

#include "resources/resource.h"
#include "typedefs.h"
#include <array>
#include <bitset>

namespace cyclonite {
class Material : public resources::Resource
{
public:
    enum class AlphaMode
    {
        OPAQUE = 0,
        MASK = 1,
        BLEND = 2,
        ADDITIVE_BLEND = 3,
        SUBTRACTIVE_BLEND = 4,
        MULTIPLY_BLEND = 5,
        CUSTOM_BLEND = 6,
        MIN_VALUE = OPAQUE,
        MAX_VALUE = CUSTOM_BLEND,
        COUNT = MAX_VALUE + 1
    };

    enum class BlendEquation
    {
        FUNC_ADD = 0,
        FUNC_SUBTRACT = 1,
        FUNC_REVERSE_SUBTRACT = 2,
        MIN_VALUE = FUNC_ADD,
        MAX_VALUE = FUNC_REVERSE_SUBTRACT,
        COUNT = MAX_VALUE + 1
    };

    enum class BlendFactor
    {
        ZERO = 0,
        ONE = 1,
        SRC_COLOR = 2,
        ONE_MINUS_SRC_COLOR = 3,
        DST_COLOR = 4,
        ONE_MINUS_DST_COLOR = 5,
        SRC_ALPHA = 6,
        ONE_MINUS_SRC_ALPHA = 7,
        DST_ALPHA = 8,
        ONE_MINUS_DST_ALPHA = 9,
        CONSTANT_COLOR = 10,
        ONE_MINUS_CONSTANT_COLOR = 11,
        CONSTANT_ALPHA = 12,
        ONE_MINUS_CONSTANT_ALPHA = 13,
        SRC_ALPHA_SATURATE = 14,
        SRC1_COLOR = 15,
        ONE_MINUS_SRC1_COLOR = 16,
        SRC1_ALPHA = 17,
        ONE_MINUS_SRC1_ALPHA = 18,
        MIN_VALUE  = ZERO,
        MAX_VALUE = ONE_MINUS_SRC1_ALPHA,
        COUNT = MAX_VALUE + 1
    };

    enum class CullFace
    {
        NONE = 0,
        FRONT = 1,
        BACK = 2,
        BOTH = 3,
        MIN_VALUE = FRONT,
        MAX_VALUE = BACK,
        COUNT = MAX_VALUE + 1
    };

    enum class PolygonMode
    {
        FILL = 0,
        LINE = 1,
        POINT = 2,
        MIN_VALUE = FILL,
        MAX_VALUE = POINT,
        COUNT = MAX_VALUE + 1
    };

    enum class Flags
    {
        DEPTH_WRITE = 0x01,
        DEPTH_TEST = 0x02,
        RASTERIZER_DISCARD_ENABLE = 0x04,
        DEPTH_CLAMP_ENABLE = 0x08,
        DEPTH_BIAS_ENABLE = 0x10,
        MIN_VALUE = DEPTH_WRITE,
        MAX_VALUE = DEPTH_BIAS_ENABLE,
        COUNT = 5
    };

private:
    std::string name_;

    CullFace cullFace_;

    std::array<AlphaMode, render_target_output_semantic_count_v> alphaModePerOutput_;
    std::array<BlendEquation, render_target_output_semantic_count_v> colorBlendEquationPerOutput_;
    std::array<BlendEquation, render_target_output_semantic_count_v> alphaBlendEquationPerOutput_;
    std::array<BlendFactor, render_target_output_semantic_count_v> colorSrcBlendFactorPerOutput_;
    std::array<BlendFactor, render_target_output_semantic_count_v> colorDstBlendFactorPerOutput_;
    std::array<BlendFactor, render_target_output_semantic_count_v> alphaSrcBlendFactorPreOutput_;
    std::array<BlendFactor, render_target_output_semantic_count_v> alphaDstBlendFactorPerOutput_;

    real alphaCutoff_;
    real lineWith_;
    real depthBiasClamp_;
    real depthBiasConstantFactor_;
    real depthBiasSlopeFactor_;

    PolygonMode polygonMode_;

    std::bitset<easy_mp::value_cast(Flags::COUNT)> flags_;
};
}

#endif // CYCLONITE_MATERIAL_H
