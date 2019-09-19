//
// Created by bantdit on 9/17/19.
//

#ifndef CYCLONITE_SDLWINDOW_H
#define CYCLONITE_SDLWINDOW_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <cassert>
#include <functional>
#include <memory>
#include <string>

namespace cyclonite::sdl {
class SDLWindow
{
public:
    SDLWindow(std::string const& title,
              int x = SDL_WINDOWPOS_UNDEFINED,
              int y = SDL_WINDOWPOS_UNDEFINED,
              int w = 512,
              int h = 512,
              uint32_t flags = SDL_WINDOW_SHOWN);

    SDLWindow(SDLWindow const&) = delete;

    SDLWindow(SDLWindow&&) = default;

    ~SDLWindow() = default;

    auto operator=(SDLWindow const&) -> SDLWindow& = delete;

    auto operator=(SDLWindow &&) -> SDLWindow& = default;

    template<typename T>
    auto get() const -> T;

private:
    SDL_SysWMinfo sysWMinfo_;
    std::unique_ptr<SDL_Window, std::function<void(SDL_Window*)>> sdlWindowPtr_;
};

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

#endif // CYCLONITE_SDLWINDOW_H