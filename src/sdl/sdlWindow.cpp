//
// Created by bantdit on 9/17/19.
//

#include "sdlWindow.h"
#include <SDL2/SDL_syswm.h>

namespace cyclonite::sdl {
SDLWindow::SDLWindow(const std::string& title,
                     int x /* = SDL_WINDOWPOS_UNDEFINED*/,
                     int y /* = SDL_WINDOWPOS_UNDEFINED*/,
                     int w /* = 512*/,
                     int h /*= 512*/,
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
}
