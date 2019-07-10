//
// Created by bantdit on 2/6/19.
//

#ifndef CYCLONITE_ATTRIBUTESEMANTIC_H
#define CYCLONITE_ATTRIBUTESEMANTIC_H

#include <cstdint>

namespace cyclonite::core {
enum class AttributeSemantic : uint8_t
{
    NONE = 0,
    POSITION = 1,
    NORMAL = 2,
    TANGENT = 3,
    BI_TANGENT = 4,
    TEX_COORD_CHANNEL_0 = 6,
    TEX_COORD_CHANNEL_1 = 7,
    TEX_COORD_CHANNEL_2 = 8,
    TEX_COORD_CHANNEL_3 = 9,
    TEX_COORD_CHANNEL_4 = 10,
    TEX_COORD_CHANNEL_5 = 11,
    TEX_COORD_CHANNEL_6 = 12,
    TEX_COORD_CHANNEL_7 = 13,
    TEX_COORD_CHANNEL_8 = 14,
    TEX_COORD_CHANNEL_9 = 15,
    COLOR_0 = 16,
    COLOR_1 = 17,
    COLOR_2 = 18,
    COLOR_3 = 19,
    COLOR_4 = 20,
    COLOR_5 = 21,
    COLOR_6 = 22,
    COLOR_7 = 23,
    COLOR_8 = 24,
    COLOR_9 = 25,
    BONE_INDICES_0 = 26,
    BONE_WEIGHTS_0 = 27,
    BONE_INDICES_1 = 28,
    BONE_WEIGHTS_1 = 29,
    BONE_INDICES_2 = 30,
    BONE_WEIGHTS_2 = 31,
    BONE_INDICES_3 = 32,
    BONE_WEIGHTS_3 = 33,
    BONE_INDICES_4 = 34,
    BONE_WEIGHTS_4 = 35,
    CUSTOM_0 = 36,
    CUSTOM_1 = 37,
    CUSTOM_2 = 38,
    CUSTOM_3 = 39,
    CUSTOM_4 = 40,
    CUSTOM_5 = 41,
    CUSTOM_6 = 42,
    CUSTOM_7 = 43,
    CUSTOM_8 = 44,
    CUSTOM_9 = 45,
    MIN_VALUE = NONE,
    MAX_VALUE = CUSTOM_9,
    COUNT = 46
};
}

#endif // CYCLONITE_ATTRIBUTESEMANTIC_H
