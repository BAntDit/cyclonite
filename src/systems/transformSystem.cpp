//
// Created by bantdit on 1/18/20.
//

#include "transformSystem.h"
#include <glm/gtx/matrix_decompose.hpp>

namespace cyclonite::systems {
void TransformSystem::init() {}

void TransformSystem::_decompose(mat4& mat, vec3& position, vec3& scale, quat& orientation)
{
    vec3 skew{};
    vec4 perspective{};

    auto success = glm::decompose(mat, scale, orientation, position, skew, perspective);

    assert(success);
}
}
