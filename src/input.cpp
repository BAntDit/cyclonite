//
// Created by bantdit on 12/31/19.
//

#include "input.h"

namespace cyclonite {
Input::Input()
  : quit{}
  , keyDown{}
  , keyUp{}
  , mouseButtonDown{}
  , mouseButtonUp{}
  , mouseMotion{}
  , mouseWheel{}
{}

void Input::pollEvent()
{
    SDL_Event event = {};

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                quit();
                break;
            case SDL_MOUSEBUTTONDOWN:
                mouseButtonDown(event.button.button, event.button.clicks, event.button.x, event.button.y);
                break;
            case SDL_MOUSEBUTTONUP:
                mouseButtonUp(event.button.button, event.button.x, event.button.y);
                break;
            case SDL_MOUSEMOTION:
                mouseMotion(event.motion.x, event.motion.y);
                break;
            case SDL_MOUSEWHEEL:
                mouseWheel(event.wheel.y);
                break;
            case SDL_KEYDOWN:
                keyDown(event.key.keysym.sym, event.key.keysym.mod);
                break;
            case SDL_KEYUP:
                keyUp(event.key.keysym.sym, event.key.keysym.mod);
                break;
            default:
                continue;
        }
    }
}
}
