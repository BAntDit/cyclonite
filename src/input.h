//
// Created by bantdit on 12/31/19.
//

#ifndef CYCLONITE_INPUT_H
#define CYCLONITE_INPUT_H

#include "event.h"
#include <SDL2/SDL.h>

namespace cyclonite {
class Input
{
public:
    Input();

    Input(Input const&) = delete;

    Input(Input&&) = delete;

    ~Input() = default;

    auto operator=(Input const&) -> Input& = delete;

    auto operator=(Input &&) -> Input& = delete;

    void pollEvent();

public:
    Event<> quit;
    Event<SDL_Keycode, uint16_t> keyDown;
    Event<SDL_Keycode, uint16_t> keyUp;
    Event<uint8_t, uint8_t, uint32_t, uint32_t> mouseButtonDown;
    Event<uint8_t, uint32_t, uint32_t> mouseButtonUp;
    Event<uint32_t, uint32_t> mouseMotion;
    Event<int32_t> mouseWheel;
};
}

#endif // CYCLONITE_INPUT_H
