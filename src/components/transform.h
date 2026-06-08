//
// Created by anton on 6/7/26.
//

#ifndef CYCLONITE_COMPONENTS_TRANSFORM_H
#define CYCLONITE_COMPONENTS_TRANSFORM_H
#include "common.h"

namespace cyclonite::components {
struct Transform
{
    enum class State : uint8_t
    {
        UPDATE_NOTHING = 0,
        UPDATE_LOCAL = 1,
        UPDATE_COMPONENTS = 2,
        UPDATE_WORLD = 4,
        MIN_VALUE = UPDATE_NOTHING,
        MAX_VALUE = UPDATE_WORLD,
        COUNT = 4
    };

    Transform();

    Transform(vec3 const& pos, vec3 const& sc, quat const& rot);

    explicit Transform(mat4 mat);

    glm::vec3 position;
    glm::vec3 scale;
    quat orientation;

    mat4 matrix;
    mat4 worldMatrix;

    State state;
};
}

#endif // CYCLONITE_COMPONENTS_TRANSFORM_H