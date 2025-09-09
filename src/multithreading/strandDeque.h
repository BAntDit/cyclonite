//
// Created by anton on 9/9/25.
//

#ifndef CYCLONITE_STRAND_DEQUE_H
#define CYCLONITE_STRAND_DEQUE_H

#include "common.h"
#include <atomic>
#include <cstdint>

namespace cyclonite::multithreading {
template<DequeItemConcept DataItemType>
class StrandDeque
{
public:
    explicit StrandDeque(size_t capacity);

    ~StrandDeque();

    [[nodiscard]] auto capacity() const -> size_t { return data_->capacity(); }

    [[nodiscard]] auto isEmpty() const -> bool;

    [[nodiscard]] auto countItems() const -> size_t;

    // can be called in any producer thread
    template<typename... Args>
    auto tryEmplace(Args&&... args) -> bool;

private:
    alignas(hardware_destructive_interference_size) std::atomic<bool> allowPop_;
    alignas(hardware_destructive_interference_size) std::atomic<uint64_t> consumerTop_;
    alignas(hardware_destructive_interference_size) std::atomic<uint64_t> consumerBottom_;
    alignas(hardware_destructive_interference_size) std::atomic<uint64_t> producerBottom_;
    alignas(hardware_destructive_interference_size) internal::DequeData<DataItemType>* data_;
};

template<DequeItemConcept DataItemType>
StrandDeque<DataItemType>::StrandDeque(size_t capacity)
  : allowPop_{ true }
  , consumerTop_{ 0 }
  , consumerBottom_{ 0 }
  , producerBottom_{ 0 }
  , data_{ new internal::DequeData<DataItemType>{ capacity } }
{
}

template<DequeItemConcept DataItemType>
StrandDeque<DataItemType>::~StrandDeque()
{
    delete data_;
}

template<DequeItemConcept DataItemType>
auto StrandDeque<DataItemType>::countItems() const -> size_t
{
    auto bottom = consumerBottom_.load(std::memory_order_acquire);
    auto top = consumerTop_.load(std::memory_order_acquire);

    return bottom - consumerTop_;
}

template<DequeItemConcept DataItemType>
auto StrandDeque<DataItemType>::isEmpty() const -> bool
{
    auto bottom = producerBottom_.load(std::memory_order_acquire);
    auto top = consumerTop_.load(std::memory_order_acquire);

    return (bottom - top) == 0;
}

template<DequeItemConcept DataItemType>
template<typename... Args>
auto StrandDeque<DataItemType>::tryEmplace(Args&&... args) -> bool
{
    if (countItems() < capacity()) {
        auto consumerBottom = consumerBottom_.load(std::memory_order_acquire);
        auto producerBottom = producerBottom_.fetch_add(1, std::memory_order_acq_rel);

        if (consumerBottom == producerBottom) {
            data_->store(producerBottom, DataItemType(std::forward<Args>(args)...));

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

#endif // CYCLONITE_STRAND_DEQUE_H
