//
// Created by anton on 6/7/26.
//

#include "transform.h"
#include <glm/gtx/matrix_decompose.hpp>

namespace cyclonite::components {
Transform::Transform()
  : position{ 0.f }
  , scale{ 1.f }
  , orientation{ glm::angleAxis(glm::radians(0.f), vec3{ 0.f, 1.f, 1.f }) }
  , matrix{ 1.f }
  , worldMatrix{ 1.f }
  , state{ State::UPDATE_NOTHING }
{
}

Transform::Transform(vec3 const& pos, vec3 const& sc, quat const& rot)
  : position(pos)
  , scale(sc)
  , orientation(rot)
  , matrix{ glm::translate(pos) * glm::mat4_cast(rot) * glm::scale(sc) }
  , worldMatrix{ 1.f }
  , state{ State::UPDATE_WORLD }
{
}

Transform::Transform(mat4 mat)
  : position{ 1.f }
  , scale{ 1.f }
  , orientation{ glm::angleAxis(glm::radians(0.f), vec3{ 0.f, 1.f, 1.f }) }
  , matrix{ mat }
  , worldMatrix{ 1.f }
  , state{ State::UPDATE_WORLD }
{
    [[maybe_unused]] vec3 skew{};
    [[maybe_unused]] vec4 perspective{};

    [[maybe_unused]] bool success = glm::decompose(matrix, scale, orientation, position, skew, perspective);
    assert(success);
}
}
