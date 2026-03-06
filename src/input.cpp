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
{
}

void Input::pollEvent()
{
    SDL_Event event = {};

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
                quit();
                break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                mouseButtonDown(event.button.button, event.button.clicks, event.button.x, event.button.y);
                break;
            case SDL_EVENT_MOUSE_BUTTON_UP:
                mouseButtonUp(event.button.button, event.button.x, event.button.y);
                break;
            case SDL_EVENT_MOUSE_MOTION:
                mouseMotion(event.motion.x, event.motion.y);
                break;
            case SDL_EVENT_MOUSE_WHEEL:
                mouseWheel(event.wheel.y);
                break;
            case SDL_EVENT_KEY_DOWN:
                keyDown(event.key.key, event.key.mod);
                break;
            case SDL_EVENT_KEY_UP:
                keyUp(event.key.key, event.key.mod);
                break;
            default:
                continue;
        }
    }
}
}
