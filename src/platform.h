//
// Created by bantdit on 9/18/19.
//

#ifndef CYCLONITE_PLATFORM_H
#define CYCLONITE_PLATFORM_H

#include <SDL3/SDL.h>

#if defined(VK_USE_PLATFORM_XLIB_KHR)
#include <X11/Xlib.h>
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
#include <wayland-client.h>
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
#include <android/native_window.h>
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
#include <windows.h>
#endif

#endif // CYCLONITE_PLATFORM_H
