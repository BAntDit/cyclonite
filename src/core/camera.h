//
// Created by bantdit on 1/27/19.
//

#ifndef CYCLONITE_PERSPECTIVECAMERA_H
#define CYCLONITE_PERSPECTIVECAMERA_H

#include "typedefs.h"

namespace cyclonite::core {
struct PerspectiveCamera
{
    explicit PerspectiveCamera(std::string const& _name = "",
                               real _near = 0.0f,
                               real _far = 10.0f,
                               real _aspect = 1.0f,
                               real _fov = 1.5708)
      : name{ _name }
      , near{ _near }
      , far{ _far }
      , aspect{ _aspect }
      , fov{ _fov }
    {}

    std::string name;

    real near;
    real far;
    real aspect;
    real fov; // vertical fov in radians
};

struct OrthographicCamera
{
    explicit OrthographicCamera(std::string const& _name = "",
                                real _near = 0.0f,
                                real _far = 2.0f,
                                real _xmag = 2.0f,
                                real _ymag = 2.0f)
      : name{ _name }
      , near{ _near }
      , far{ _far }
      , xmag{ _xmag }
      , ymag{ _ymag }
    {}

    std::string name;

    real near;
    real far;
    real xmag;
    real ymag;
};
}
// PerspectiveCamera

#endif // CYCLONITE_PERSPECTIVECAMERA_H
