//
// Created by anton on 4/25/21.
//

#ifndef CYCLONITE_PASSTYPE_H
#define CYCLONITE_PASSTYPE_H

namespace cyclonite::compositor {
enum class PassType
{
    SCENE = 0,
    SCREEN = 1,
    MIN_VALUE = SCENE,
    MAX_VALUE = SCREEN,
    COUNT = MAX_VALUE + 1
};
}

#endif // CYCLONITE_PASSTYPE_H
