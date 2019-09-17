//
// Created by bantdit on 9/15/19.
//

#include "sdlSupport.h"
#include <stdexcept>

namespace cyclonite::sdl {
SDLSupport::SDLSupport()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        throw std::runtime_error("could not initialize SDL video subsystem");
    }
}

SDLSupport::~SDLSupport()
{
    SDL_Quit();
}

void SDLSupport::storeDisplayResolutions(Options& options, int displayIndex)
{
    auto countDisplayModes = SDL_GetNumDisplayModes(displayIndex);

    for (auto i = 0; i < countDisplayModes; i++) {
        SDL_DisplayMode displayMode = {};

        SDL_GetDisplayMode(displayIndex, i, &displayMode);

        auto width = static_cast<uint16_t>(displayMode.w);
        auto height = static_cast<uint16_t>(displayMode.h);

        options.displayResolutions().emplace_back(width, height);
    }
}

auto SDLSupport::createWindow(std::string const& title,
                              int left /* = SDL_WINDOWPOS_UNDEFINED*/,
                              int top /* = SDL_WINDOWPOS_UNDEFINED*/,
                              int width /* = 512*/,
                              int height /* = 512*/,
                              uint32_t flags /* = SDL_WINDOW_SHOWN*/) -> SDLWindow
{
    return SDLWindow(title, left, top, width, height, flags);
}
}
