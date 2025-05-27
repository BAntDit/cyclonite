//
// Created by bantdit on 9/17/19.
//

#include "sdlWindow.h"
#include "platform.h"
#include <SDL3/SDL_properties.h>
#include <SDL3/SDL_video.h>
#include <stdexcept>

namespace cyclonite::sdl {
SDLWindow::SDLWindow(const std::string& title,
                     unsigned int w /* = 512*/,
                     unsigned int h /*= 512*/,
                     uint32_t flags /* = 0*/)
  : sdlWindowPtr_(SDL_CreateWindow(title.c_str(), w, h, flags), [](SDL_Window* window) { SDL_DestroyWindow(window); })
{
    if (!sdlWindowPtr_) {
        throw std::runtime_error(SDL_GetError());
    }
}

#if defined(VK_USE_PLATFORM_XLIB_KHR)
template<>
auto SDLWindow::get<Display*>() const -> Display*
{
    assert(SDL_strcmp(SDL_GetCurrentVideoDriver(), "x11") == 0);
    return static_cast<Display*>(SDL_GetPointerProperty(
      SDL_GetWindowProperties(sdlWindowPtr_.get()), SDL_PROP_WINDOW_X11_DISPLAY_POINTER, nullptr));
}

template<>
auto SDLWindow::get<Window>() const -> Window
{
    assert(SDL_strcmp(SDL_GetCurrentVideoDriver(), "x11") == 0);
    return static_cast<Window>(
      SDL_GetNumberProperty(SDL_GetWindowProperties(sdlWindowPtr_.get()), SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0));
}
#endif

#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
template<>
auto SDLWindow::get<wl_display*>() const -> wl_display*
{
    assert(SDL_strcmp(SDL_GetCurrentVideoDriver(), "wayland") == 0);
    return static_cast<wl_display*>(SDL_GetPointerProperty(
      SDL_GetWindowProperties(sdlWindowPtr_.get()), SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER, nullptr));
}

template<>
auto SDLWindow::get<wl_surface*>() const -> wl_surface*
{
    assert(SDL_strcmp(SDL_GetCurrentVideoDriver(), "wayland") == 0);
    return static_cast<wl_surface*>(SDL_GetPointerProperty(
      SDL_GetWindowProperties(sdlWindowPtr_.get()), SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER, nullptr));
}
#endif

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
template<>
auto SDLWindow::get<ANativeWindow*>() const -> ANativeWindow*
{
    assert(SDL_strcmp(SDL_GetCurrentVideoDriver(), "android") == 0);
    return static_cast<ANativeWindow*>(SDL_GetPointerProperty(
      SDL_GetWindowProperties(sdlWindowPtr_.get()), SDL_PROP_WINDOW_ANDROID_WINDOW_POINTER, nullptr));
}
#endif

#if defined(VK_USE_PLATFORM_WIN32_KHR)
template<>
auto SDLWindow::get<HWND>() const -> HWND
{
    assert(SDL_strcmp(SDL_GetCurrentVideoDriver(), "windows") == 0);
    return static_cast<HWND>(SDL_GetPointerProperty(
      SDL_GetWindowProperties(sdlWindowPtr_.get()), SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr));
}

template<>
auto SDLWindow::get<HINSTANCE>() const -> HINSTANCE
{
    assert(SDL_strcmp(SDL_GetCurrentVideoDriver(), "windows") == 0);
    return static_cast<HINSTANCE>(SDL_GetPointerProperty(
      SDL_GetWindowProperties(sdlWindowPtr_.get()), SDL_PROP_WINDOW_WIN32_INSTANCE_POINTER, nullptr));
}
#endif
}
