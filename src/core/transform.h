//
// Created by bantdit on 1/25/19.
//

#ifndef CYCLONITE_TRANSFORM_H
#define CYCLONITE_TRANSFORM_H

#include "typedefs.h"
#include <glm/gtx/matrix_decompose.hpp>

namespace cyclonite::core {
struct Transform
{
    explicit Transform(mat4 const& _matrix)
      : position{}
      , scale{}
      , orientation{}
      , matrix{}
      , worldMatrix{ 1.0 }
    {
        vec3 skew{};
        vec4 perspective{};

        auto success = glm::decompose(_matrix, scale, orientation, position, skew, perspective);

        matrix = glm::transpose(_matrix);

        assert(success);
    }

    explicit Transform(mat3x4 const& _matrix)
      : position{}
      , scale{}
      , orientation{}
      , matrix{ _matrix }
      , worldMatrix{ 1.0 }
    {
        mat4 transform = glm::transpose(matrix);

        vec3 skew{};
        vec4 perspective{};

        auto success = glm::decompose(transform, scale, orientation, position, skew, perspective);

        assert(success);
    }

    explicit Transform(vec3 const& _position = vec3{},
                       vec3 const& _scale = vec3{ 1.0 },
                       quat const& _orientation = quat{})
      : position{ _position }
      , scale{ _scale }
      , orientation{ _orientation }
      , matrix{}
      , worldMatrix{ 1.0 }
    {
        mat4 transform = glm::translate(position) * glm::mat4_cast(orientation) * glm::scale(scale);

        matrix = glm::transpose(transform);
    }

    vec3 position;
    vec3 scale;
    quat orientation;

    mat3x4 matrix;
    mat3x4 worldMatrix;
};
}

#endif // CYCLONITE_TRANSFORM_H
