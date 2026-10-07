//
// Created by anton on 10/7/26.
//

#ifndef CYCLONITE_MT_DYNAMIC_MPSC_QUEUE_H
#define CYCLONITE_MT_DYNAMIC_MPSC_QUEUE_H

#include "multithreading/common.h"
#include <atomic>
#include <optional>

namespace cyclonite::multithreading {
// Dmitry Vyukov's MPSC queue implementation
template<typename ItemType>
class DynamicMpscQueue
{
    struct Node
    {
        std::atomic<Node*> next;
        std::optional<ItemType> value;
    };

public:
    DynamicMpscQueue();

    ~DynamicMpscQueue();

    // can call producer thread only
    template<typename... Args>
    auto emplaceBack(Args&&... args) -> ItemType&;

    // can call consumer thread only
    auto popFront() -> std::optional<ItemType>;

    [[nodiscard]] auto isEmpty() const -> bool { return tail_->next.load(std::memory_order_acquire) == nullptr; }

private:
    alignas(hardware_destructive_interference_size) std::atomic<Node*> head_;
    alignas(hardware_destructive_interference_size) Node* tail_;
};

template<typename ItemType>
DynamicMpscQueue<ItemType>::DynamicMpscQueue()
  : head_{ nullptr }
  , tail_{ nullptr }
{
    auto* sentinelNode = new Node{};
    head_.store(sentinelNode, std::memory_order_relaxed);
    tail_ = sentinelNode;
}

template<typename ItemType>
DynamicMpscQueue<ItemType>::~DynamicMpscQueue()
{
    auto* node = tail_;
    while (node != nullptr) {
        auto* next = node->next.load(std::memory_order_relaxed);
        delete node;
        node = next;
    }
}

template<typename ItemType>
template<typename... Args>
auto DynamicMpscQueue<ItemType>::emplaceBack(Args&&... args) -> ItemType&
{
    auto* node = new Node{};
    auto& r = node->value.emplace(std::forward<Args>(args)...);
    Node* prev = head_.exchange(node, std::memory_order_acq_rel);
    prev->next.store(node, std::memory_order_release);

    return r;
}

template<typename ItemType>
auto DynamicMpscQueue<ItemType>::popFront() -> std::optional<ItemType>
{
    Node* tail = tail_;
    Node* next = tail->next.load(std::memory_order_acquire);

    if (next == nullptr)
        return std::nullopt;

    auto r = std::optional<ItemType>(std::move(next->value));
    next->value.reset();
    tail_ = next;
    delete tail;

    return r;
}
}

#endif // CYCLONITE_MT_DYNAMIC_MPSC_QUEUE_H