//
// Created by bantdit on 9/17/19.
//

#include "sdlWindow.h"
#include <SDL2/SDL_syswm.h>
#include <stdexcept>

namespace cyclonite::sdl {
SDLWindow::SDLWindow(const std::string& title,
                     unsigned int x /* = SDL_WINDOWPOS_UNDEFINED*/,
                     unsigned int y /* = SDL_WINDOWPOS_UNDEFINED*/,
                     unsigned int w /* = 512*/,
                     unsigned int h /*= 512*/,
                     uint32_t flags /* = SDL_WINDOW_SHOWN*/)
  : sysWMinfo_{}
  , sdlWindowPtr_(SDL_CreateWindow(title.c_str(), x, y, w, h, flags),
                  [](SDL_Window* window) { SDL_DestroyWindow(window); })
{
    if (!sdlWindowPtr_) {
        throw std::runtime_error(SDL_GetError());
    }

    SDL_VERSION(&sysWMinfo_.version);

    if (SDL_GetWindowWMInfo(sdlWindowPtr_.get(), &sysWMinfo_) != SDL_TRUE) {
        throw std::runtime_error("could not get window WM Info");
    }
}

#if defined(SDL_VIDEO_DRIVER_X11)
template<>
auto SDLWindow::get<Display*>() const -> Display*
{
    assert(sysWMinfo_.subsystem == SDL_SYSWM_X11);
    return sysWMinfo_.info.x11.display;
}

template<>
auto SDLWindow::get<Window const&>() const -> Window const&
{
    assert(sysWMinfo_.subsystem == SDL_SYSWM_X11);
    return sysWMinfo_.info.x11.window;
}
#endif

#if defined(SDL_VIDEO_DRIVER_WAYLAND)
template<>
auto SDLWindow::get<wl_display*>() const -> wl_display*
{
    assert(sysWMinfo_.subsystem == SDL_SYSWM_WAYLAND);
    return sysWMinfo_.info.wl.display;
}

template<>
auto SDLWindow::get<wl_surface*>() const -> wl_surface*
{
    assert(sysWMinfo_.subsystem == SDL_SYSWM_WAYLAND);
    return sysWMinfo_.info.wl.surface;
}

template<>
auto SDLWindow::get<wl_shell_surface*>() const -> wl_shell_surface*
{
    assert(sysWMinfo_.subsystem == SDL_SYSWM_WAYLAND);
    return sysWMinfo_.info.wl.shell_surface;
}
#endif

#if defined(SDL_VIDEO_DRIVER_ANDROID)
template<>
auto SDLWindow::get<ANativeWindow*>() const -> ANativeWindow*
{
    assert(sysWMinfo_.subsystem == SDL_SYSWM_ANDROID);
    return sysWMinfo_.info.android.window;
}

template<>
auto SDLWindow::get<EGLSurface>() const -> EGLSurface const&
{
    assert(sysWMinfo_.subsystem == SDL_SYSWM_ANDROID);
    return sysWMinfo_.info.android.surface;
}
#endif

#if defined(SDL_VIDEO_DRIVER_WINDOWS)
template<>
auto SDLWindow::get<HWND>() const -> HWND
{
    assert(sysWMinfo_.subsystem == SDL_SYSWM_WINDOWS);
    return sysWMinfo_.info.win.window;
}

template<>
auto SDLWindow::get<HDC>() const -> HDC
{
    assert(sysWMinfo_.subsystem == SDL_SYSWM_WINDOWS);
    return sysWMinfo_.info.win.hdc;
}

template<>
auto SDLWindow::get<HINSTANCE>() const -> HINSTANCE
{
    assert(sysWMinfo_.subsystem == SDL_SYSWM_WINDOWS);
    return sysWMinfo.info.win.hinstance;
}
#endif
}
