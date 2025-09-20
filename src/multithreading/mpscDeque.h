//
// Created by anton on 9/8/25.
//

#ifndef CYCLONITE_MPSC_DEQUE_H
#define CYCLONITE_MPSC_DEQUE_H

#include "common.h"
#include <atomic>
#include <cstdint>
#include <limits>
#include <optional>

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

    // can be called in consumer thread only
    auto tryPop() -> std::optional<DataItemType>;

private:
    alignas(hardware_destructive_interference_size) std::atomic<uint64_t> consumerTop_;
    alignas(hardware_destructive_interference_size) std::atomic<uint64_t> consumerBottom_;
    alignas(hardware_destructive_interference_size) std::atomic<uint64_t> producerBottom_;
    alignas(hardware_destructive_interference_size) internal::DequeData<DataItemType>* data_;
};

template<DequeItemConcept DataItemType>
MpscDeque<DataItemType>::MpscDeque(size_t capacity)
  : consumerTop_{ 0 }
  , consumerBottom_{ 0 }
  , producerBottom_{ 0 }
  , data_{ new internal::DequeData<DataItemType>{ capacity } }
{
}

template<DequeItemConcept DataItemType>
MpscDeque<DataItemType>::~MpscDeque()
{
    delete data_;
}

template<DequeItemConcept DataItemType>
auto MpscDeque<DataItemType>::countItems() const -> size_t
{
    auto bottom = consumerBottom_.load(std::memory_order_acquire);
    auto top = consumerTop_.load(std::memory_order_acquire);

    return static_cast<size_t>((bottom >= top) ? bottom - top : (std::numeric_limits<uint64_t>::max() - top + bottom));
}

template<DequeItemConcept DataItemType>
auto MpscDeque<DataItemType>::isEmpty() const -> bool
{
    auto bottom = producerBottom_.load(std::memory_order_acquire);
    auto top = consumerTop_.load(std::memory_order_acquire);

    return (bottom - top) == 0;
}

template<DequeItemConcept DataItemType>
template<typename... Args>
auto MpscDeque<DataItemType>::tryEmplace(Args&&... args) -> bool
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

template<DequeItemConcept DataItemType>
auto MpscDeque<DataItemType>::tryPop() -> std::optional<DataItemType>
{
    auto result = std::optional<DataItemType>{ std::nullopt };

    auto consumerBottom = consumerBottom_.load(std::memory_order_acquire);
    auto consumerTop = consumerTop_.load(std::memory_order_acquire);

    if (consumerTop != consumerBottom) {
        result = data_->load(consumerTop);
        consumerTop_.fetch_add(1, std::memory_order_release);
    }

    return result;
}
}

#endif // CYCLONITE_MPSC_DEQUE_H
