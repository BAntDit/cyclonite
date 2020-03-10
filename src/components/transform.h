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
        UP_TO_DATE = 0,
        NEEDS_UPDATE_LOCAL_MATRIX = 1,
        NEEDS_UPDATE_COMPONENTS = 2,
        NEEDS_UPDATE_WORLD_MATRIX = 3,
        MIN_VALUE = UP_TO_DATE,
        MAX_VALUE = NEEDS_UPDATE_WORLD_MATRIX,
        COUNT = MAX_VALUE + 1
    };

    Transform() noexcept;

    Transform(vec3 localPosition, vec3 localScale, quat localOrientation) noexcept;

    explicit Transform(mat4 localMatrix);

    vec3 position;
    vec3 scale;
    quat orientation;
    mat4 matrix;

    State state;

    enttx::Entity parent;

    size_t depth;
    size_t globalIndex;
};
}

#endif // CYCLONITE_TRANSFORM_H
