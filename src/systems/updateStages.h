//
// Created by bantdit on 1/28/19.
//

#ifndef CYCLONITE_UPDATESTAGES_H
#define CYCLONITE_UPDATESTAGES_H

#include <cstddef>

namespace cyclonite {
enum class UpdateStage : size_t
{
    EARLY_UPDATE = 0,
    LATE_UPDATE = 1,
    TRANSFER_STAGE = 2,
    RENDERING = 3,
    MIN_VALUE = EARLY_UPDATE,
    MAX_VALUE = RENDERING,
    COUNT = MAX_VALUE + 1
};
}

#endif // CYCLONITE_UPDATESTAGES_H
