//
// Created by bantdit on 1/25/19.
//

#ifndef CYCLONITE_TRANSFORM_H
#define CYCLONITE_TRANSFORM_H

#include "typedefs.h"

namespace cyclonite::core
{
    struct Transform
    {
        explicit Transform(vec3 _position = vec3{}, vec3 _scale = vec3{1.0}, quat _orientation = quat{}) :
            position{_position}
            , scale{_scale}
            , orientation{_orientation}
            , matrix{}
        {}

        vec3 position;
        vec3 scale;
        quat orientation;

        mat3x4 matrix;
    };
}

#endif //CYCLONITE_TRANSFORM_H
