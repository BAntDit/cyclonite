//
// Created by bantdit on 1/24/19.
//

#ifndef CYCLONITE_TYPEDEFS_H
#define CYCLONITE_TYPEDEFS_H

#include <boost/cstdfloat.hpp>
#include <cstdint>
#include <easy-mp/enum.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>

namespace cyclonite {
enum class OutputChannel
{
    R = 1,
    G = 2,
    B = 4,
    A = 8,
    MIN_VALUE = R,
    MAX_VALUE = A,
    COUNT = 4
};

inline constexpr auto channel_output_count_v = easy_mp::value_cast(OutputChannel::COUNT);

enum class ShaderStage
{
    VERTEX_STAGE = 0,
    TESSELATION_CONTROL_STAGE = 1,
    TESSELATION_EVALUATION_STAGE = 2,
    GEOMETRY_STAGE = 3,
    FRAGMENT_STAGE = 4,
    COMPUTE_STAGE = 5,
    MESHLETS_PIPELINE_TASK_STAGE = 6,
    MESHLETS_PIPELINE_MESH_STAGE = 7,
    RAY_TRACING_RAY_GENERATION_STAGE = 8,
    RAY_TRACING_INTERSECTION_STAGE = 9,
    RAY_TRACING_ANY_HIT_STAGE = 10,
    RAY_TRACING_CLOSEST_HIT_STAGE = 11,
    RAY_TRACING_MISS_STAGE = 12,
    MIN_VALUE = VERTEX_STAGE,
    MAX_VALUE = RAY_TRACING_MISS_STAGE,
    COUNT = MAX_VALUE + 1,
    UNDEFINED = COUNT,
    FIRST_RASTERIZATION_STAGE = VERTEX_STAGE,
    LAST_RASTERIZATION_STAGE = FRAGMENT_STAGE,
    RASTERIZATION_STAGE_COUNT = FRAGMENT_STAGE - VERTEX_STAGE,
};

inline constexpr auto shader_stage_count_v = easy_mp::value_cast(ShaderStage::COUNT);

inline constexpr auto rasterization_shader_stage_count_v = easy_mp::value_cast(ShaderStage::RASTERIZATION_STAGE_COUNT);

enum class RenderTargetOutputSemantic : size_t
{
    UNDEFINED = 0,
    DEFAULT = 1,
    VIEW_SPACE_NORMALS = 2,
    ALBEDO = 3,
    LINEAR_HDR_COLOR = 4,
    FINAL_SRGB_COLOR = 6,
    MIN_VALUE = UNDEFINED,
    MAX_VALUE = FINAL_SRGB_COLOR,
    COUNT = MAX_VALUE + 1,
    INVALID = COUNT
};

inline constexpr auto render_target_output_semantic_count_v = easy_mp::value_cast(RenderTargetOutputSemantic::COUNT);

enum class InterpolationType : uint8_t
{
    STEP = 0,
    LINEAR = 1,
    SPHERICAL = 2,
    CUBIC = 3,
    CATMULL_ROM = 4,
    MIN_VALUE = STEP,
    MAX_VALUE = CATMULL_ROM,
    COUNT = MAX_VALUE + 1
};

enum class InterpolationElementType
{
    ARRAY = 0,
    SCALAR = 1,
    VEC2 = 2,
    VEC3 = 3,
    VEC4 = 4,
    MAT2 = 5,
    MAT3 = 6,
    MAT4 = 7,
    MAT3x2 = 8,
    MAT2x3 = 9,
    MAT3x4 = 10,
    MAT4x3 = 11,
    QUAT = 12,
    MIN_VALUE = ARRAY,
    MAX_VALUE = QUAT,
    COUNT = MAX_VALUE + 1
};

using real = boost::float32_t;

using uint = std::uint32_t;

using vec2 = glm::tvec2<boost::float32_t, glm::highp>;

using vec3 = glm::tvec3<boost::float32_t, glm::highp>;

using vec4 = glm::tvec4<boost::float32_t, glm::highp>;

using uvec2 = glm::tvec2<std::uint32_t, glm::highp>;

using uvec3 = glm::tvec3<std::uint32_t, glm::highp>;

using uvec4 = glm::tvec4<std::uint32_t, glm::highp>;

using quat = glm::tquat<boost::float32_t, glm::highp>;

using mat3x4 = glm::tmat3x4<boost::float32_t, glm::highp>; // 3x4 => C = 3, R = 4

using mat3 = glm::tmat3x3<boost::float32_t, glm::highp>; // 3x3

using mat4 = glm::tmat4x4<boost::float32_t, glm::highp>; // 4x4

// just for now - fixed vertex and instanced data layouts :(
struct Vertex
{
    alignas(sizeof(vec4)) vec3 position;
    alignas(sizeof(vec4)) vec3 normal;
};

// static_assert(sizeof(Vertex) == sizeof(boost::float32_t) * 6);

struct InstancedData
{
    vec4 transform1;
    vec4 transform2;
    vec4 transform3;
};

using vertex_t = Vertex;

using instanced_data_t = InstancedData;

using index_type_t = uint32_t;

#ifdef __cpp_lib_hardware_interference_size
using std::hardware_constructive_interference_size;
using std::hardware_destructive_interference_size;
#else
// https://en.cppreference.com/w/cpp/thread/hardware_destructive_interference_size
// 64 bytes on x86-64 │ L1_CACHE_BYTES │ L1_CACHE_SHIFT │ __cacheline_aligned │ ...
inline constexpr size_t hardware_constructive_interference_size = 64;
inline constexpr size_t hardware_destructive_interference_size = 64;
#endif
}

#endif // CYCLONITE_TYPEDEFS_H
