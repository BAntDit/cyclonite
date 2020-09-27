//
// Created by bantdit on 1/28/19.
//

#ifndef CYCLONITE_UPDATESTAGES_H
#define CYCLONITE_UPDATESTAGES_H

#include <cstddef>

namespace cyclonite {
enum class UpdateStage : size_t
{
    FRAME_START = 0,
    EARLY_UPDATE = 1,
    LATE_UPDATE = 2,
    TRANSFER_STAGE = 3,
    RENDERING = 4,
    FRAME_END = 5,
    MIN_VALUE = FRAME_START,
    MAX_VALUE = FRAME_END,
    COUNT = MAX_VALUE + 1
};
}

#endif // CYCLONITE_UPDATESTAGES_H
