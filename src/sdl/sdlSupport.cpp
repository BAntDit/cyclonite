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

void SDLSupport::storeDisplayResolutions(std::vector<std::pair<uint16_t, uint16_t>>& displayResolutions,
                                         SDL_DisplayID displayId)
{
    auto displayModeCount = int32_t{0};
    auto** displayModes = SDL_GetFullscreenDisplayModes(displayId, &displayModeCount);

    if (displayModes != nullptr && displayModeCount > 0) {
        displayResolutions.reserve(displayModeCount);

        for (auto i = 0; i < displayModeCount; i++) {
            auto const& displayMode = *(displayModes[i]);

            auto width = static_cast<uint16_t>(displayMode.w);
            auto height = static_cast<uint16_t>(displayMode.h);

            displayResolutions.emplace_back(width, height);
        }
    } else {
        throw std::runtime_error("could not get available display modes");
    }
}
}
