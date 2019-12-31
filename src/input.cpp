//
// Created by bantdit on 12/31/19.
//

#include "input.h"

namespace cyclonite {
Input::Input()
  : keyDown{}
  , keyUp{}
{}

void Input::pollEvent()
{
    SDL_Event event = {};

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_KEYDOWN:
                keyDown(event.key);
                break;
            case SDL_KEYUP:
                keyUp(event.key);
                break;
            default:
                continue;
        }
    }
}
}
