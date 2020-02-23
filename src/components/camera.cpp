//
// Created by bantdit on 2/23/20.
//

#include "camera.h"

namespace cyclonite::components {
Camera::Camera(PerspectiveProjection const& perspective) noexcept
  : projection{ perspective }
{}

Camera::Camera(OrthographicProjection const& orthographic) noexcept
  : projection{ orthographic }
{}
}
