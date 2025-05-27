//
// Created by bantdit on 9/17/19.
//

#ifndef CYCLONITE_SDLWINDOW_H
#define CYCLONITE_SDLWINDOW_H

#include <SDL3/SDL.h>
#include <SDL3/SDL_video.h>
#include <cassert>
#include <functional>
#include <memory>
#include <string>

namespace cyclonite::sdl {
class SDLWindow
{
public:
    SDLWindow(std::string const& title, unsigned int w = 512, unsigned int h = 512, uint32_t flags = 0);

    SDLWindow(SDLWindow const&) = delete;

    SDLWindow(SDLWindow&&) = default;

    ~SDLWindow() = default;

    auto operator=(SDLWindow const&) -> SDLWindow& = delete;

    auto operator=(SDLWindow&&) -> SDLWindow& = default;

    template<typename T>
    auto get() const -> T;

private:
    std::unique_ptr<SDL_Window, std::function<void(SDL_Window*)>> sdlWindowPtr_;
};
}

#endif // CYCLONITE_SDLWINDOW_H
