//
// Created by bantdit on 12/29/22.
//

#ifndef CYCLONITE_MULTITHREADING_UTILS_H
#define CYCLONITE_MULTITHREADING_UTILS_H

#include <random>

namespace cyclonite::multithreading::internal {
static auto randomWorkerIndex(size_t count) -> size_t
{
    assert(count > 0);

    static thread_local auto rd = std::random_device();
    static thread_local auto generator = std::mt19937{ rd() };

    std::uniform_int_distribution<int> distribution(0, static_cast<int>(count) - 1);

    return static_cast<size_t>(distribution(generator));
}
}

#endif // CYCLONITE_MULTITHREADING_UTILS_H
