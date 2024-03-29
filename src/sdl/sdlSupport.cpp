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

void SDLSupport::storeDisplayResolutions(std::vector<std::pair<uint16_t, uint16_t>>& displayREsoltions,
                                         int displayIndex)
{
    auto countDisplayModes = SDL_GetNumDisplayModes(displayIndex);

    displayREsoltions.reserve(countDisplayModes);

    for (auto i = 0; i < countDisplayModes; i++) {
        SDL_DisplayMode displayMode = {};

        SDL_GetDisplayMode(displayIndex, i, &displayMode);

        auto width = static_cast<uint16_t>(displayMode.w);
        auto height = static_cast<uint16_t>(displayMode.h);

        displayREsoltions.emplace_back(width, height);
    }
}
}
