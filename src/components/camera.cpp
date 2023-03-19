//
// Created by bantdit on 2/23/20.
//

#include "camera.h"

namespace cyclonite::components {
Camera::Camera() noexcept
  : projection{ Camera::PerspectiveProjection{ 1.0f, 1.5708f, 0.1f, 10.0f } }
{
}

Camera::Camera(PerspectiveProjection const& perspective) noexcept
  : projection{ perspective }
{
}

Camera::Camera(OrthographicProjection const& orthographic) noexcept
  : projection{ orthographic }
{
}
}
