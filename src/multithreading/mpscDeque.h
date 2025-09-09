//
// Created by anton on 9/8/25.
//

#ifndef CYCLONITE_MPSC_DEQUE_H
#define CYCLONITE_MPSC_DEQUE_H

#include "common.h"
#include <atomic>
#include <cstdint>

namespace cyclonite::multithreading {
template<DequeItemConcept DataItemType>
class MpscDeque
{
public:
    explicit MpscDeque(size_t capacity);

    ~MpscDeque();

    [[nodiscard]] auto capacity() const -> size_t { return data_->capacity(); }

    [[nodiscard]] auto isEmpty() const -> bool;

    [[nodiscard]] auto countItems() const -> size_t;

    // can be called in any producer thread
    template<typename... Args>
    auto tryEmplace(Args&&... args) -> bool;

private:
    alignas(hardware_destructive_interference_size) std::atomic<uint64_t> consumerTop_;
    alignas(hardware_destructive_interference_size) std::atomic<uint64_t> consumerBottom_;
    alignas(hardware_destructive_interference_size) std::atomic<uint64_t> producerBottom_;
    alignas(hardware_destructive_interference_size) DequeData<DataItemType>* data_;
};

template<DequeItemConcept DataItemType>
template<typename... Args>
auto MpscDeque<DataItemType>::tryEmplace(Args&&... args) -> bool
{
    if (countItems() < capacity()) {
        auto consumerBottom = consumerBottom_.load(std::memory_order_acquire);
        auto producerBottom = producerBottom_.fetch_add(1, std::memory_order_acq_rel);

        if (consumerBottom == producerBottom) {
            data_->store(producerBottom, MpscDeque(std::forward<Args>(args)...));

            if (consumerBottom_.compare_exchange_weak(
                  consumerBottom, producerBottom + 1, std::memory_order_release, std::memory_order_relaxed)) {
                return true;
            }
        }

        producerBottom_.fetch_sub(1, std::memory_order_acq_rel);
    }

    return false;
}
}

#endif // CYCLONITE_MPSC_DEQUE_H
