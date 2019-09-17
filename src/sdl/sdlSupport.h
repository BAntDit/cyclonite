//
// Created by bantdit on 9/15/19.
//

#ifndef CYCLONITE_SDLSUPPORT_H
#define CYCLONITE_SDLSUPPORT_H

#include "../options.h"
#include "sdlWindow.h"
#include <SDL2/SDL.h>

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

    [[nodiscard]] auto createWindow(std::string const& title,
                                    int left = SDL_WINDOWPOS_UNDEFINED,
                                    int top = SDL_WINDOWPOS_UNDEFINED,
                                    int width = 512,
                                    int height = 512,
                                    uint32_t flags = SDL_WINDOW_SHOWN) const -> SDLWindow;
};
}

#endif // CYCLONITE_SDLSUPPORT_H
