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
class Task;

class StrandDeque
{
public:
    explicit StrandDeque(size_t capacity);

    ~StrandDeque();

    [[nodiscard]] auto capacity() const -> size_t { return data_->capacity(); }

    [[nodiscard]] auto isEmpty() const -> bool;

    [[nodiscard]] auto countItems() const -> size_t;

    // can be called in any producer thread
    auto tryEmplace(Task* task) -> bool;

    auto tryPop() -> std::optional<Task*>;

private:
    void allowPop() { allowPop_.store(true, std::memory_order_release); }

    alignas(hardware_destructive_interference_size) std::atomic<bool> allowPop_;
    alignas(hardware_destructive_interference_size) std::atomic<uint64_t> consumerTop_;
    alignas(hardware_destructive_interference_size) std::atomic<uint64_t> consumerBottom_;
    alignas(hardware_destructive_interference_size) std::atomic<uint64_t> producerBottom_;
    alignas(hardware_destructive_interference_size) internal::DequeData<Task*>* data_;
};
}

#endif // CYCLONITE_STRAND_DEQUE_H
