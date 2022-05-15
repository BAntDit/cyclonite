//
// Created by bantdit on 1/12/20.
//

#ifndef CYCLONITE_TRANSFORM_H
#define CYCLONITE_TRANSFORM_H

#include "../typedefs.h"
#include <enttx/enttx.h>

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

    Transform() noexcept;

    Transform(vec3 localPosition, vec3 localScale, quat localOrientation) noexcept;

    explicit Transform(mat4 localMatrix);

    vec3 position;
    vec3 scale;
    quat orientation;
    mat4 matrix;
    mat4 worldMatrix;

    State state;

    enttx::Entity parent;

    size_t depth;
};
}

#endif // CYCLONITE_TRANSFORM_H
