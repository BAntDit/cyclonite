//
// Created by anton on 9/9/25.
//

#ifndef CYCLONITE_STRAND_DEQUE_H
#define CYCLONITE_STRAND_DEQUE_H

#include "common.h"
#include <atomic>
#include <cstdint>
#include <optional>

namespace cyclonite::multithreading {
class StrandTask;

class StrandDeque
{
public:
    explicit StrandDeque(size_t capacity);

    ~StrandDeque();

    [[nodiscard]] auto capacity() const -> size_t { return data_->capacity(); }

    [[nodiscard]] auto isEmpty() const -> bool;

    [[nodiscard]] auto countItems() const -> size_t;

    // can be called in any producer thread
    auto tryEmplace(StrandTask* task) -> bool;

    auto tryPop() -> std::optional<StrandTask*>;

private:
    alignas(hardware_destructive_interference_size) std::atomic<bool> allowPop_;
    alignas(hardware_destructive_interference_size) std::atomic<uint64_t> consumerTop_;
    alignas(hardware_destructive_interference_size) std::atomic<uint64_t> consumerBottom_;
    alignas(hardware_destructive_interference_size) std::atomic<uint64_t> producerBottom_;
    alignas(hardware_destructive_interference_size) internal::DequeData<StrandTask*>* data_;
};
}

#endif // CYCLONITE_STRAND_DEQUE_H
