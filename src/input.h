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
    Event<SDL_KeyboardEvent> keyDown;
    Event<SDL_KeyboardEvent> keyUp;
};
}

#endif // CYCLONITE_INPUT_H
