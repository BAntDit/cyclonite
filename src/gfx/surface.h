//
// Created by anton on 6/11/25.
//

#ifndef GFX_SURFACE_H
#define GFX_SURFACE_H

#include "./interfaces/surfaceInterface.h"
#include "./vulkan/vkSurface.h"

namespace cyclonite::gfx
{
using Surface = interfaces::SurfaceInterface<vulkan::Surface>;
}

#endif // GFX_SURFACE_H
