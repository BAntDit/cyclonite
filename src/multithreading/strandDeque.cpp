//
// Created by anton on 9/12/25.
//

#include "strandDeque.h"
#include "task.h"
#include <limits>

namespace cyclonite::multithreading {
StrandDeque::StrandDeque(size_t capacity)
  : allowPop_{ true }
  , consumerTop_{ 0 }
  , consumerBottom_{ 0 }
  , producerBottom_{ 0 }
  , data_{ new internal::DequeData<Task*>{ capacity } }
{
}

StrandDeque::~StrandDeque()
{
    delete data_;
}

auto StrandDeque::countItems() const -> size_t
{
    auto bottom = consumerBottom_.load(std::memory_order_acquire);
    auto top = consumerTop_.load(std::memory_order_acquire);

    return static_cast<size_t>((bottom >= top) ? bottom - top : (std::numeric_limits<uint64_t>::max() - top + bottom));
}

auto StrandDeque::isEmpty() const -> bool
{
    auto bottom = producerBottom_.load(std::memory_order_acquire);
    auto top = consumerTop_.load(std::memory_order_acquire);

    return (bottom - top) == 0;
}

auto StrandDeque::tryEmplace(Task* task) -> bool
{
    if (countItems() < capacity()) {
        auto consumerBottom = consumerBottom_.load(std::memory_order_acquire);
        auto producerBottom = producerBottom_.fetch_add(1, std::memory_order_acq_rel);

        if (consumerBottom == producerBottom) {
            data_->store(producerBottom, std::add_pointer_t<Task>{ task });

            if (consumerBottom_.compare_exchange_weak(
                  consumerBottom, producerBottom + 1, std::memory_order_release, std::memory_order_relaxed)) {
                return true;
            }
        }

        producerBottom_.fetch_sub(1, std::memory_order_acq_rel);
    }

    return false;
}

auto StrandDeque::tryPop() -> std::optional<Task*>
{
    auto result = std::optional<Task*>{ std::nullopt };
    auto expected = true;

    if (allowPop_.compare_exchange_weak(expected, false, std::memory_order_acq_rel, std::memory_order_relaxed)) {
        auto bottom = consumerBottom_.load(std::memory_order_acquire);
        auto top = consumerTop_.load(std::memory_order_relaxed);

        if (bottom != top) {
            assert(top == consumerTop_.load(std::memory_order_acquire)); // cano t be changed in other thread
            consumerTop_.store(top + 1, std::memory_order_release);
            result = data_->load(top);
        } else {
            allowPop_.store(true, std::memory_order_release);
        }
    }

    return result;
}
}
