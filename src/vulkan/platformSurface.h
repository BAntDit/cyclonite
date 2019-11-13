//
// Created by bantdit on 11/13/19.
//

#ifndef CYCLONITE_PLATFORMSURFACE_H
#define CYCLONITE_PLATFORMSURFACE_H

#include "../platform.h"

#if defined(VK_USE_PLATFORM_XLIB_KHR)
#include "xlibSurface.h"
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
#include "wlSurface.h"
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
#include "androidSurface.h"
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
#include "win32Surface.h"
#endif

#endif // CYCLONITE_PLATFORMSURFACE_H
