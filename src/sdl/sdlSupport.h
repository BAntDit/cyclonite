//
// Created by bantdit on 9/15/19.
//

#ifndef CYCLONITE_SDLSUPPORT_H
#define CYCLONITE_SDLSUPPORT_H

#include <SDL2/SDL.h>
#include "../options.h"

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

    auto operator=(SDLSupport &&) -> SDLSupport& = delete;

public:
    void storeDisplayResolutions(Options& options, int displayIndex);
};
}

#endif // CYCLONITE_SDLSUPPORT_H
