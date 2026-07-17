
#ifndef CYCLONITE_COMMON_H
#define CYCLONITE_COMMON_H

#include <boost/cstdfloat.hpp>
#include <glm/detail/type_quat.hpp>
#include <glm/glm.hpp>

namespace cyclonite {
using real = boost::float32_t;

using vec2 = glm::tvec2<boost::float32_t, glm::highp>;

using vec3 = glm::tvec3<boost::float32_t, glm::highp>;

using vec4 = glm::tvec4<boost::float32_t, glm::highp>;

using quat = glm::tquat<boost::float32_t, glm::highp>;

using mat3x4 = glm::tmat3x4<boost::float32_t, glm::highp>; // 3x4 => C = 3, R = 4

using mat3 = glm::tmat3x3<boost::float32_t, glm::highp>; // 3x3

using mat4 = glm::tmat4x4<boost::float32_t, glm::highp>; // 4x4
}

#endif // CYCLONITE_COMMON_H
