
#ifndef CYCLONITE_COMMON_H
#define CYCLONITE_COMMON_H

#include <boost/cstdfloat.hpp>
#include <glm/glm.hpp>


namespace cyclonite {
using real = boost::float32_t;

using vec2 = glm::tvec2<boost::float32_t, glm::highp>;

using vec3 = glm::tvec3<boost::float32_t, glm::highp>;

using vec4 = glm::tvec4<boost::float32_t, glm::highp>;

using quat = glm::tquat<boost::float32_t, glm::highp>;
}

#endif // CYCLONITE_COMMON_H
