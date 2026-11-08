//
// Created by anton on 11/8/26.
//

#ifndef CYCLONITE_COMPOSITION_COMMON_H
#define CYCLONITE_COMPOSITION_COMMON_H

#include <cstdint>

namespace cyclonite::composition {
enum class RenderPassType : uint8_t
{
    SCENE = 0,
    SCREEN = 1
};
}

#endif // CYCLONITE_COMPOSITION_COMMON_H
