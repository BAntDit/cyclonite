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
    MAX_VALUE = LATE_UPDATE,
    COUNT = 2
};
}

#endif // CYCLONITE_UPDATESTAGES_H
