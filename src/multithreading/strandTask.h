//
// Created by anton on 9/12/25.
//

#ifndef CYCLONITE_STRAND_TASK_H
#define CYCLONITE_STRAND_TASK_H

#include "common.h"
#include <atomic>

namespace cyclonite::multithreading {
class StrandTask;
class StrandDeque;

template<typename T>
concept TaskFunctorConcept = !
std::is_same_v<std::decay_t<T>, StrandTask>;

class alignas(hardware_destructive_interference_size) StrandTask
{
public:
    StrandTask();

private:
    alignas(
      internal::storage_align_v) std::byte storage_[internal::storage_size_v]; // for small functor optimization (sfo)
    internal::functor_base_t* functor_;
    std::atomic<bool> pending_;
    StrandDeque* deque_;
};
}

#endif // CYCLONITE_STRAND_TASK_H
