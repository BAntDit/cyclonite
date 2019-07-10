//
// Created by bantdit on 5/27/19.

#ifndef CYCLONITE_ATTRIBUTESEMANTIC_H
#define CYCLONITE_ATTRIBUTESEMANTIC_H

#include <cstddef>

namespace cyclonite::core {
enum class AttributeSemantic : size_t
{
    INDEX = 0,
    POSITION = 1,
    NORMAL = 2,
    TANGENT = 3,
    BITANGENT = 4,
    TEX_COORD_CHANNEL_0 = 5,
    TEX_COORD_CHANNEL_1 = 6,
    TEX_COORD_CHANNEL_2 = 7,
    TEX_COORD_CHANNEL_3 = 8,
    TEX_COORD_CHANNEL_4 = 9,
    TEX_COORD_CHANNEL_5 = 10,
    COLOR_0 = 11,
    COLOR_1 = 12,
    COLOR_2 = 13,
    COLOR_3 = 14,
    COLOR_4 = 15,
    COLOR_5 = 16,
    BONE_INDICES_0 = 17,
    BONE_WEIGHTS_0 = 18,
    BONE_INDICES_1 = 19,
    BONE_WEIGHTS_1 = 20,
    CUSTOM_0 = 21,
    CUSTOM_1 = 22,
    CUSTOM_2 = 23,
    CUSTOM_3 = 24,
    CUSTOM_4 = 25,
    CUSTOM_5 = 26,
    CUSTOM_6 = 27,
    CUSTOM_7 = 28,
    MIN_VALUE = POSITION,
    MAX_VALUE = CUSTOM_7,
    COUNT = MAX_VALUE + 1
};
}

#endif // CYCLONITE_ATTRIBUTESEMANTIC_H
