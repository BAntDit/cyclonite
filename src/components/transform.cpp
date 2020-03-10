//
// Created by bantdit on 1/12/20.
//

#include "transform.h"
#include <glm/gtx/matrix_decompose.hpp>

namespace cyclonite::components {
Transform::Transform() noexcept
  : position{ 0.0f }
  , scale{ 1.0f }
  , orientation{ glm::angleAxis(glm::radians(0.0f), vec3{ 0.0f, 1.0f, 1.0f }) }
  , matrix{ 1.0f }
  , state{ State::UP_TO_DATE }
  , parent{}
  , depth{ 0 }
  , globalIndex{ std::numeric_limits<size_t>::max() }
{}

Transform::Transform(vec3 localPosition, vec3 localScale, quat localOrientation) noexcept
  : position{ localPosition }
  , scale{ localScale }
  , orientation{ localOrientation }
  , matrix{ glm::translate(localPosition) * glm::mat4_cast(localOrientation) * glm::scale(localScale) } // TRS
  , state{ State::NEEDS_UPDATE_LOCAL_MATRIX }
  , parent{}
  , depth{ 0 }
  , globalIndex{ std::numeric_limits<size_t>::max() }
{}

Transform::Transform(mat4 localMatrix)
  : position{ 0.0f }
  , scale{ 1.0f }
  , orientation{ glm::angleAxis(glm::radians(0.0f), vec3{ 0.0f, 1.0f, 1.0f }) }
  , matrix{ localMatrix }
  , state{ State::NEEDS_UPDATE_COMPONENTS }
  , parent{}
  , depth{ 0 }
  , globalIndex{ std::numeric_limits<size_t>::max() }
{
    vec3 skew{};
    vec4 perspective{};

    bool success = glm::decompose(matrix, scale, orientation, position, skew, perspective);

    assert(success);

    (void)success;
}
}
