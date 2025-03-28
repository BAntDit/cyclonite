//
// Created by bantdit on 10/27/22.
//

#ifndef CYCLONITE_TASK_H
#define CYCLONITE_TASK_H

#include "typedefs.h"
#include <atomic>
#include <functional>

namespace cyclonite::multithreading {
class Task;

template<typename F>
concept TaskFunctor = requires(F&& f)
{
    // !std::is_same_v<std::decay_t<F>, Task> &&
    std::invoke(std::forward<F>(f));
};

class alignas(hardware_constructive_interference_size) Task
{
    static constexpr size_t storage_size_v = 64;

    using storage_t = std::array<std::byte, 64>;

    struct functor_base_t
    {
        functor_base_t() = default;

        virtual ~functor_base_t() = default;

        virtual void invoke() = 0;
        virtual auto move_to(storage_t& storage) -> functor_base_t* = 0;
    };

    template<typename F>
    struct functor_t : functor_base_t
    {
        explicit functor_t(F const& f)
          : f_(f)
        {}

        explicit functor_t(F&& f)
          : f_(std::move(f))
        {}

        void invoke() override { f_(); };

        auto move_to(storage_t& storage) -> functor_base_t* override;

    private:
        F f_;
    };

public:
    Task();

    template<TaskFunctor F>
    explicit Task(F&& f);

    Task(Task const&) = delete;

    Task(Task&& task) noexcept;

    auto operator=(Task const&) -> Task& = delete;

    auto operator=(Task&& rhs) noexcept -> Task&;

    void operator()();

    [[nodiscard]] auto pending() const -> bool { return pending_.load(std::memory_order_acquire); }

    ~Task();

private:
    void _reset();

    auto storage() -> void* { return storage_.data(); }

private:
    storage_t storage_;
    functor_base_t* functor_;
    std::atomic<bool> pending_;
};

template<TaskFunctor F>
Task::Task(F&& f)
  : storage_{}
  , functor_{ nullptr }
  , pending_{ false }
{
    if constexpr (sizeof(functor_t<F>) <= sizeof(storage_t)) {
        functor_ = new (storage()) functor_t<F>{ std::forward<F>(f) };
    } else {
        functor_ = new functor_t<F>{ std::forward<F>(f) };
    }

    pending_.store(functor_ != nullptr, std::memory_order_relaxed);
}

template<typename F>
auto Task::functor_t<F>::move_to(storage_t& storage) -> functor_base_t*
{
    functor_base_t* r = nullptr;

    if constexpr (sizeof(f_) <= sizeof(storage_t)) {
        r = new (storage.data()) functor_t<std::decay_t<decltype(f_)>>{ std::move(f_) };
    }

    assert(r != nullptr);
    return r;
}
}

#endif // CYCLONITE_TASK_H
