//
// Created by bantdit on 7/10/19.
//

#ifndef CYCLONITE_PRIMITIVETYPE_H
#define CYCLONITE_PRIMITIVETYPE_H

#include <cstddef>

namespace cyclonite::core {
enum class PrimitiveType : size_t
{
    POINT_LIST = 0,
    LINE_LIST = 1,
    LINE_STRIP = 2,
    TRIANGLE_LIST = 3,
    TRIANGLE_STRIP = 4,
    TRIANGLE_FAN = 5,
    LINE_LIST_WITH_ADJACENCY = 6,
    LINE_STRIP_WITH_ADJACENCY = 7,
    TRIANGLE_LIST_WITH_ADJACENCY = 8,
    TRIANGLE_STRIP_WITH_ADJACENCY = 9,
    PATCH_LIST = 10,
    MIN_VALUE = POINT_LIST,
    MAX_VALUE = PATCH_LIST,
    COUNT = MAX_VALUE + 1
};
}

#endif // CYCLONITE_PRIMITIVETYPE_H
