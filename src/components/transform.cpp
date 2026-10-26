//
// Created by anton on 6/7/26.
//

#include "transform.h"
#include <glm/gtx/matrix_decompose.hpp>

namespace cyclonite::components {
Transform::Transform()
  : parent_{}
  , position_{ 0.f }
  , scale_{ 1.f }
  , orientation_{ glm::angleAxis(glm::radians(0.f), vec3{ 0.f, 1.f, 1.f }) }
  , matrix_{ 1.f }
  , worldMatrix_{ 1.f }
  , level_{ std::numeric_limits<uint32_t>::max() }
  , state_{ State::UPDATE_NOTHING }
{
}

Transform::Transform(enttx::Entity parent, uint32_t level)
  : Transform{}
{
    parent_ = parent;
    level_ = level;
}

Transform::Transform(enttx::Entity parent, uint32_t level, vec3 const& pos, vec3 const& sc, quat const& rot)
  : Transform{ parent, level }
{
    position_ = pos;
    scale_ = sc;
    orientation_ = rot;
    matrix_ = glm::translate(pos) * glm::mat4_cast(rot) * glm::scale(sc);
    state_ = State::UPDATE_WORLD;
}

Transform::Transform(enttx::Entity parent, uint32_t level, mat4 mat)
  : Transform{ parent, level }
{
    matrix_ = mat;

    [[maybe_unused]] vec3 skew{};
    [[maybe_unused]] vec4 perspective{};

    [[maybe_unused]] bool success = glm::decompose(matrix_, scale_, orientation_, position_, skew, perspective);
    assert(success);

    state_ = State::UPDATE_WORLD;
}

void Transform::setPosition(vec3 const& pos)
{
    position_ = pos;
    state_ = State::UPDATE_LOCAL;
}

void Transform::setScale(vec3 const& sc)
{
    scale_ = sc;
    state_ = State::UPDATE_LOCAL;
}

void Transform::setOrientation(quat const& rotation)
{
    orientation_ = rotation;
    state_ = State::UPDATE_LOCAL;
}

void Transform::setMatrix(mat4 const& mat)
{
    matrix_ = mat;
    state_ = State::UPDATE_LOCAL;
}

void Transform::setWorldMatrix(mat4 const& worldMatrix)
{
    worldMatrix_ = worldMatrix;
    state_ = State::UPDATE_CHILDREN;
}
}
