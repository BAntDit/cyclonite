//
// Created by bantdit on 1/12/20.
//

#ifndef CYCLONITE_TRANSFORM_H
#define CYCLONITE_TRANSFORM_H

#include "../typedefs.h"
#include <bitset>

namespace cyclonite::components {
class TransformSystem;

struct Transform
{
    friend class TransformSystem;

    Transform() noexcept;

    Transform(vec3 localPosition, vec3 localScale, quat localOrientation) noexcept;

    explicit Transform(mat4 localMatrix);

    [[nodiscard]] auto depth() const -> size_t { return depth_; }

private:
    vec3 position;
    vec3 scale;
    quat orientation;
    mat4 matrix;

    std::bitset<3> flags;

    size_t depth_;
    size_t globalIndex;
    size_t parentIndex;
};
}

#endif // CYCLONITE_TRANSFORM_H
