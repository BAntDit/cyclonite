//
// Created by bantdit on 9/18/19.
//

#ifndef CYCLONITE_PLATFORM_H
#define CYCLONITE_PLATFORM_H

#include <SDL2/SDL.h>

// TODO:: fix undefined platform
#if defined(SDL_VIDEO_DRIVER_X11)
#define VK_USE_PLATFORM_XLIB_KHR
#elif defined(SDL_VIDEO_DRIVER_WAYLAND)
#define VK_USE_PLATFORM_WAYLAND_KHR
#elif define(SDL_VIDEO_DRIVER_ANDROID)
#define VK_USE_PLATFORM_ANDROID_KHR
#elif defined(SDL_VIDEO_DRIVER_WINDOWS)
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#endif // CYCLONITE_PLATFORM_H
