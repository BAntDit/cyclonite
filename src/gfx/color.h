
#ifndef CYCLONITE_GFX_COLOR
#define CYCLONITE_GFX_COLOR

// #include "cyclonite/common.h"

#include "gfx/common.h"

namespace cyclonite::gfx {
struct Color
{
    Color() = default;

    Color(real colr, real colg, real colb, real cola = 1.0f)
      : r{ colr }
      , g{ colg }
      , b{ colb }
      , a{ cola }
    {
    }

    explicit operator vec4() const { return vec4{ r, g, b, a }; };

    real r;
    real g;
    real b;
    real a;
};
}

#endif // CYCLONITE_GFX_COLOR
