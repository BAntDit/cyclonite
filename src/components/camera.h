//
// Created by bantdit on 2/23/20.
//

#ifndef CYCLONITE_CAMERA_H
#define CYCLONITE_CAMERA_H

#include "typedefs.h"
#include <cstddef>
#include <variant>

namespace cyclonite::components {
struct Camera
{
    struct PerspectiveProjection
    {
        PerspectiveProjection(real aspectRatio, real fov, real near, real far)
          : aspect{ aspectRatio }
          , yFov{ fov }
          , zNear{ near }
          , zFar{ far }
        {}

        real aspect;
        real yFov;
        real zNear;
        real zFar;
    };

    struct OrthographicProjection
    {
        OrthographicProjection(real xMagnification, real yMagnification, real near, real far)
          : xMag{ xMagnification }
          , yMag{ yMagnification }
          , zNear{ near }
          , zFar{ far }
        {}

        real xMag;
        real yMag;
        real zNear;
        real zFar;
    };

    Camera(PerspectiveProjection const& perspective) noexcept;

    Camera(OrthographicProjection const& orthographic) noexcept;

    std::variant<PerspectiveProjection, OrthographicProjection> projection;
};
}

#endif // CYCLONITE_CAMERA_H
