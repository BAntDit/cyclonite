//
// Created by bantdit on 9/17/19.
//

#ifndef CYCLONITE_SDLWINDOW_H
#define CYCLONITE_SDLWINDOW_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
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

    [[nodiscard]] auto display() const -> Display*;

    [[nodiscard]] auto window() const -> Window;

private:
    SDL_SysWMinfo sysWMinfo_;
    std::unique_ptr<SDL_Window, std::function<void(SDL_Window*)>> sdlWindowPtr_;
};
}

#endif // CYCLONITE_SDLWINDOW_H
