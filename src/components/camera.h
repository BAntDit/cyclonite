//
// Created by anton on 6/7/26.
//

#ifndef CYCLONITE_COMPONENTS_CAMERA_H
#define CYCLONITE_COMPONENTS_CAMERA_H

#include "common.h"
#include <variant>

namespace cyclonite::components {
struct Camera
{
    struct PerspectiveProjection
    {
        PerspectiveProjection(real aspectRatio, real fov, real n, real f)
          : aspect{ aspectRatio }
          , yFov{ fov }
          , zNear{ n }
          , zFar{ f }
        {
        }

        real aspect;
        real yFov;
        real zNear;
        real zFar;
    };

    struct OrthographicProjection
    {
        OrthographicProjection(real xMagnification, real yMagnification, real n, real f)
          : xMag{ xMagnification }
          , yMag{ yMagnification }
          , zNear{ n }
          , zFar{ f }
        {
        }

        real xMag;
        real yMag;
        real zNear;
        real zFar;
    };

    Camera() noexcept
      : projection{ Camera::PerspectiveProjection{ 1.0f, 1.5708f, 0.1f, 10.f } }
    {
    }

    explicit Camera(PerspectiveProjection const& perspective) noexcept
      : projection{ perspective }
    {
    }

    explicit Camera(OrthographicProjection const& orthographic) noexcept
      : projection{ orthographic }
    {
    }

    std::variant<PerspectiveProjection, OrthographicProjection> projection;
};
}

#endif // CYCLONITE_COMPONENTS_CAMERA_H