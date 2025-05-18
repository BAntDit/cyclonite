//
// Created by bantdit on 9/15/19.
//

#ifndef CYCLONITE_SDLSUPPORT_H
#define CYCLONITE_SDLSUPPORT_H

#include "../options.h"
#include "sdlWindow.h"

namespace cyclonite::sdl {
class SDLSupport
{
public:
    SDLSupport();

    SDLSupport(SDLSupport const&) = delete;

    SDLSupport(SDLSupport&&) = delete;

    ~SDLSupport();

public:
    auto operator=(SDLSupport const&) -> SDLSupport& = delete;

    auto operator=(SDLSupport&&) -> SDLSupport& = delete;

public:
    void storeDisplayResolutions(std::vector<std::pair<uint16_t, uint16_t>>& displayResolutions,
                                 SDL_DisplayID displayId);
};
}

#endif // CYCLONITE_SDLSUPPORT_H
