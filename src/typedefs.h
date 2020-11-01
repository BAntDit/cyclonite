//
// Created by bantdit on 1/24/19.
//

#ifndef CYCLONITE_TYPEDEFS_H
#define CYCLONITE_TYPEDEFS_H

#include <boost/cstdfloat.hpp>
#include <cstdint>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>

namespace cyclonite {
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
}

#endif // CYCLONITE_TYPEDEFS_H
