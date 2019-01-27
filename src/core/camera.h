//
// Created by bantdit on 1/27/19.
//

#ifndef CYCLONITE_PERSPECTIVECAMERA_H
#define CYCLONITE_PERSPECTIVECAMERA_H

#include "typedefs.h"

namespace cyclonite::core
{
    struct PerspectiveCamera
    {
        real near;
        real far;
        real aspect;
        real fov; // vertical fov in radians
    };

    struct OrthographicCamera
    {
        real near;
        real far;
        real left;
        real right;
        real top;
        real bottom;
    };
}
// PerspectiveCamera

#endif //CYCLONITE_PERSPECTIVECAMERA_H
