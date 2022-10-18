//
// Created by anton on 7/5/22.
//

#ifndef CYCLONITE_INTEROLATION_H
#define CYCLONITE_INTEROLATION_H

#include "typedefs.h"
#include <glm/gtc/type_ptr.hpp>
#include <immintrin.h>

namespace cyclonite::animations::internal {
inline void step_scalar_interpolation(real alpha, real, uint8_t count, real const* src, real* dst)
{
    auto const* a = src;
    auto const* b = a + count;

    *dst = alpha > 0.5f ? *b : *a;
}

inline void linear_scalar_interpolation(real alpha, real, uint8_t count, real const* src, real* dst)
{
    auto const* a = src;
    auto const* b = a + count;

    *dst = (1.f - alpha) * (*a) + alpha * (*b);
}

inline void cubic_spline_scalar_interpolation(real alpha, real delta, uint8_t count, real const* src, real* dst)
{
    auto const* a = src;
    auto const* b = src + count + count;

    auto a2 = alpha * alpha;
    auto a3 = a2 * alpha;

    auto s2 = -2.f * a3 + 3.f * a2;
    auto s3 = a3 - a2;
    auto s0 = 1.f - s2;
    auto s1 = s3 - a2 + alpha;

    auto p0 = *a;                   // vertex k
    auto m0 = *(a + count) * delta; // out tangent k * (t_k+1 - t_k)
    auto p1 = *b;                   // vertex k+1
    auto m1 = *(b + count) * delta; // in tangent k+1 * (t_k+1 - t_k)

    *dst = s0 * p0 + s1 * m0 + s2 * p1 + s3 * m1;
}

inline void slerp_interpolation(real alpha, real, uint8_t count, real const* src, real* dst)
{
    assert(count == 4);

    auto a = glm::make_quat(src);
    auto b = glm::make_quat(src + count);

    std::copy_n(glm::value_ptr(glm::slerp(a, b, alpha)), count, dst);
}

template<InterpolationType interpolationType, size_t... I>
inline void scalar_interpolation_N(real alpha,
                                   real delta,
                                   uint8_t count,
                                   real const* src,
                                   real* dst,
                                   std::index_sequence<I...>)
{
    if constexpr (interpolationType == InterpolationType::STEP)
        (step_scalar_interpolation(alpha, delta, count, src + I, dst + I), ...);

    if constexpr (interpolationType == InterpolationType::LINEAR)
        (linear_scalar_interpolation(alpha, delta, count, src + I, dst + I), ...);

    if constexpr (interpolationType == InterpolationType::CUBIC)
        (cubic_spline_scalar_interpolation(alpha, delta, count, src + I, dst + I), ...);
}

inline void step_mat2_interpolation(real alpha, real delta, uint8_t count, real const* src, real* dst)
{
    scalar_interpolation_N<InterpolationType::STEP>(alpha, delta, count, src, dst, std::make_index_sequence<4>{});
}

inline void step_mat3_interpolation(real alpha, real delta, uint8_t count, real const* src, real* dst)
{
    scalar_interpolation_N<InterpolationType::STEP>(alpha, delta, count, src, dst, std::make_index_sequence<9>{});
}

inline void step_mat4_interpolation(real alpha, real delta, uint8_t count, real const* src, real* dst)
{
    scalar_interpolation_N<InterpolationType::STEP>(alpha, delta, count, src, dst, std::make_index_sequence<16>{});
}

inline void step_mat2x3_interpolation(real alpha, real delta, uint8_t count, real const* src, real* dst)
{
    scalar_interpolation_N<InterpolationType::STEP>(alpha, delta, count, src, dst, std::make_index_sequence<6>{});
}

inline void step_mat3x2_interpolation(real alpha, real delta, uint8_t count, real const* src, real* dst)
{
    scalar_interpolation_N<InterpolationType::STEP>(alpha, delta, count, src, dst, std::make_index_sequence<6>{});
}

inline void step_mat4x3_interpolation(real alpha, real delta, uint8_t count, real const* src, real* dst)
{
    scalar_interpolation_N<InterpolationType::STEP>(alpha, delta, count, src, dst, std::make_index_sequence<12>{});
}

inline void step_mat3x4_interpolation(real alpha, real delta, uint8_t count, real const* src, real* dst)
{
    scalar_interpolation_N<InterpolationType::STEP>(alpha, delta, count, src, dst, std::make_index_sequence<12>{});
}

inline void step_vec2_interpolation(real alpha, real delta, uint8_t count, real const* src, real* dst)
{
    scalar_interpolation_N<InterpolationType::STEP>(alpha, delta, count, src, dst, std::make_index_sequence<2>{});
}

inline void step_vec3_interpolation(real alpha, real delta, uint8_t count, real const* src, real* dst)
{
    scalar_interpolation_N<InterpolationType::STEP>(alpha, delta, count, src, dst, std::make_index_sequence<3>{});
}

inline void step_vec4_interpolation(real alpha, real delta, uint8_t count, real const* src, real* dst)
{
    scalar_interpolation_N<InterpolationType::STEP>(alpha, delta, count, src, dst, std::make_index_sequence<4>{});
}

inline void step_quat_interpolation(real alpha, real delta, uint8_t count, real const* src, real* dst)
{
    scalar_interpolation_N<InterpolationType::STEP>(alpha, delta, count, src, dst, std::make_index_sequence<4>{});
}

inline void linear_vec2_interpolation(real alpha, real delta, uint8_t count, real const* src, real* dst)
{
    scalar_interpolation_N<InterpolationType::LINEAR>(alpha, delta, count, src, dst, std::make_index_sequence<2>{});
}

inline void linear_vec3_interpolation(real alpha, real delta, uint8_t count, real const* src, real* dst)
{
    scalar_interpolation_N<InterpolationType::LINEAR>(alpha, delta, count, src, dst, std::make_index_sequence<3>{});
}

inline void linear_vec4_interpolation(real alpha, real delta, uint8_t count, real const* src, real* dst)
{
    scalar_interpolation_N<InterpolationType::LINEAR>(alpha, delta, count, src, dst, std::make_index_sequence<4>{});
}

inline void linear_mat2_interpolation(real alpha, real delta, uint8_t count, real const* src, real* dst)
{
    scalar_interpolation_N<InterpolationType::LINEAR>(alpha, delta, count, src, dst, std::make_index_sequence<4>{});
}

inline void linear_mat3_interpolation(real alpha, real delta, uint8_t count, real const* src, real* dst)
{
    scalar_interpolation_N<InterpolationType::LINEAR>(alpha, delta, count, src, dst, std::make_index_sequence<9>{});
}

inline void linear_mat4_interpolation(real alpha, real delta, uint8_t count, real const* src, real* dst)
{
    scalar_interpolation_N<InterpolationType::LINEAR>(alpha, delta, count, src, dst, std::make_index_sequence<16>{});
}

inline void linear_mat3x2_interpolation(real alpha, real delta, uint8_t count, real const* src, real* dst)
{
    scalar_interpolation_N<InterpolationType::LINEAR>(alpha, delta, count, src, dst, std::make_index_sequence<6>{});
}

inline void linear_mat2x3_interpolation(real alpha, real delta, uint8_t count, real const* src, real* dst)
{
    scalar_interpolation_N<InterpolationType::LINEAR>(alpha, delta, count, src, dst, std::make_index_sequence<6>{});
}

inline void linear_mat3x4_interpolation(real alpha, real delta, uint8_t count, real const* src, real* dst)
{
    scalar_interpolation_N<InterpolationType::LINEAR>(alpha, delta, count, src, dst, std::make_index_sequence<12>{});
}

inline void linear_mat4x3_interpolation(real alpha, real delta, uint8_t count, real const* src, real* dst)
{
    scalar_interpolation_N<InterpolationType::LINEAR>(alpha, delta, count, src, dst, std::make_index_sequence<12>{});
}

inline void cubic_spline_vec2_interpolation(real alpha, real delta, uint8_t count, real const* src, real* dst)
{
    scalar_interpolation_N<InterpolationType::CUBIC>(alpha, delta, count, src, dst, std::make_index_sequence<2>{});
}

inline void cubic_spline_vec3_interpolation(real alpha, real delta, uint8_t count, real const* src, real* dst)
{
    scalar_interpolation_N<InterpolationType::CUBIC>(alpha, delta, count, src, dst, std::make_index_sequence<3>{});
}

inline void cubic_spline_vec4_interpolation(real alpha, real delta, uint8_t count, real const* src, real* dst)
{
    scalar_interpolation_N<InterpolationType::CUBIC>(alpha, delta, count, src, dst, std::make_index_sequence<4>{});
}

inline auto get_step_interpolator(InterpolationElementType interpolationElementType)
{
    using interpolator_func_t = void (*)(real alpha, real delta, uint8_t count, real const* src, real* dst);
    interpolator_func_t interpolator_func = nullptr;

    switch (interpolationElementType) {
        case InterpolationElementType::ARRAY:
            throw std::runtime_error("not implemented yet");
            break;
        case InterpolationElementType::SCALAR:
            interpolator_func = step_scalar_interpolation;
            break;
        case InterpolationElementType::VEC2:
            interpolator_func = step_vec2_interpolation;
            break;
        case InterpolationElementType::VEC3:
            interpolator_func = step_vec3_interpolation;
            break;
        case InterpolationElementType::VEC4:
            interpolator_func = step_vec4_interpolation;
            break;
        case InterpolationElementType::MAT2:
            interpolator_func = step_mat2_interpolation;
            break;
        case InterpolationElementType::MAT3:
            interpolator_func = step_mat3_interpolation;
            break;
        case InterpolationElementType::MAT4:
            interpolator_func = step_mat4_interpolation;
            break;
        case InterpolationElementType::MAT2x3:
            interpolator_func = step_mat2x3_interpolation;
            break;
        case InterpolationElementType::MAT3x2:
            interpolator_func = step_mat3x2_interpolation;
            break;
        case InterpolationElementType::MAT3x4:
            interpolator_func = step_mat3x4_interpolation;
            break;
        case InterpolationElementType::MAT4x3:
            interpolator_func = step_mat4x3_interpolation;
            break;
        case InterpolationElementType::QUAT:
            interpolator_func = step_quat_interpolation;
            break;
        default:
            assert(false);
    }

    return interpolator_func;
}

inline auto get_linear_interpolator(InterpolationElementType interpolationElementType)
{
    using interpolator_func_t = void (*)(real alpha, real delta, uint8_t count, real const* src, real* dst);
    interpolator_func_t interpolator_func = nullptr;

    switch (interpolationElementType) {
        case InterpolationElementType::ARRAY:
            throw std::runtime_error("not implemented yet");
            break;
        case InterpolationElementType::SCALAR:
            interpolator_func = linear_scalar_interpolation;
            break;
        case InterpolationElementType::VEC2:
            interpolator_func = linear_vec2_interpolation;
            break;
        case InterpolationElementType::VEC3:
            interpolator_func = linear_vec3_interpolation;
            break;
        case InterpolationElementType::VEC4:
            interpolator_func = linear_vec4_interpolation;
            break;
        case InterpolationElementType::MAT2:
            interpolator_func = linear_mat2_interpolation;
            break;
        case InterpolationElementType::MAT3:
            interpolator_func = linear_mat3_interpolation;
            break;
        case InterpolationElementType::MAT4:
            interpolator_func = linear_mat4_interpolation;
            break;
        case InterpolationElementType::MAT2x3:
            interpolator_func = linear_mat2x3_interpolation;
            break;
        case InterpolationElementType::MAT3x2:
            interpolator_func = linear_mat3x2_interpolation;
            break;
        case InterpolationElementType::MAT3x4:
            interpolator_func = linear_mat3x4_interpolation;
            break;
        case InterpolationElementType::MAT4x3:
            interpolator_func = linear_mat4x3_interpolation;
            break;
        case InterpolationElementType::QUAT:
            // must be slerp instead
        default:
            assert(false);
    }

    return interpolator_func;
}

inline auto get_cubic_interpolator(InterpolationElementType interpolationElementType)
{
    using interpolator_func_t = void (*)(real alpha, real delta, uint8_t count, real const* src, real* dst);
    interpolator_func_t interpolator_func = nullptr;

    switch (interpolationElementType) {
        case InterpolationElementType::ARRAY:
            throw std::runtime_error("not implemented yet");
            break;
        case InterpolationElementType::SCALAR:
            interpolator_func = cubic_spline_scalar_interpolation;
            break;
        case InterpolationElementType::VEC2:
            interpolator_func = cubic_spline_vec2_interpolation;
            break;
        case InterpolationElementType::VEC3:
            interpolator_func = cubic_spline_vec3_interpolation;
            break;
        case InterpolationElementType::VEC4:
            interpolator_func = cubic_spline_vec4_interpolation;
            break;
        case InterpolationElementType::MAT2:
        case InterpolationElementType::MAT3:
        case InterpolationElementType::MAT4:
        case InterpolationElementType::MAT2x3:
        case InterpolationElementType::MAT3x2:
        case InterpolationElementType::MAT3x4:
        case InterpolationElementType::MAT4x3:
        case InterpolationElementType::QUAT:
            // cubic spline interpolation has no sense for matrices
        default:
            assert(false);
    }

    return interpolator_func;
}

inline auto get_interpolator(InterpolationType interpolationType, InterpolationElementType interpolationElementType)
{
    using interpolator_func_t = void (*)(real alpha, real delta, uint8_t count, real const* src, real* dst);
    interpolator_func_t interpolator_func = nullptr;

    switch (interpolationType) {
        case InterpolationType::STEP:
            interpolator_func = get_step_interpolator(interpolationElementType);
            break;
        case InterpolationType::LINEAR:
            interpolator_func = get_linear_interpolator(interpolationElementType);
            break;
        case InterpolationType::SPHERICAL:
            assert(interpolationElementType == InterpolationElementType::QUAT);
            interpolator_func = slerp_interpolation;
            break;
        case InterpolationType::CUBIC:
            interpolator_func = get_cubic_interpolator(interpolationElementType);
            break;
        case InterpolationType::CATMULL_ROM:
            throw std::runtime_error("not implemented yet");
            break;
        default:
            assert(false);
    }

    return interpolator_func;
}
}

#endif // CYCLONITE_INTEROLATION_H
