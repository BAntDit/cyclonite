//
// Created by bantdit on 10/27/22.
//

#ifndef CYCLONITE_TASK_H
#define CYCLONITE_TASK_H

#include "typedefs.h"

namespace cyclonite::multithreading {
class alignas(hardware_constructive_interference_size) Task
{
    static constexpr size_t storage_size_v = 64;

    using storage_t = std::aligned_storage_t<storage_size_v>;

    struct functor_base_t
    {
        functor_base_t() = default;

        virtual ~functor_base_t() = default;

        virtual void invoke() = 0;
        virtual auto clone(storage_t& storage) -> functor_base_t* = 0;
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

        auto clone(storage_t& storage) -> functor_base_t* override;

        auto move_to(storage_t& storage) -> functor_base_t* override;

    private:
        F f_;
    };

public:
    template<typename F>
    Task(F&& f);

    Task(Task const& task);

    Task(Task&& task) noexcept;

    auto operator=(Task const& rhs) -> Task&;

    auto operator=(Task&& rhs) noexcept -> Task&;

    int operator()();

    ~Task();

private:
    void _reset();

    auto storage() -> void* { return std::addressof(storage_); }

private:
    storage_t storage_;
    functor_base_t* functor_;
};

template<typename F>
Task::Task(F&& f)
  : storage_{}
  , functor_{ nullptr }
{
    if constexpr (sizeof(functor_t<F>) <= sizeof(storage_t)) {
        functor_ = new (storage()) functor_t<F>{ std::forward<F>(f) };
    } else {
        functor_ = new functor_t<F>{ std::forward<F>(f) };
    }
}

template<typename F>
auto Task::functor_t<F>::clone(storage_t& storage) -> functor_base_t*
{
    functor_base_t* r = nullptr;

    if constexpr (sizeof(f_) <= sizeof(storage_t)) {
        r = new (std::addressof(storage)) functor_t<std::decay_t<decltype(f_)>>{ f_ };
    } else {
        r = new functor_t<std::decay_t<decltype(f_)>>{ f_ };
    }

    return r;
}

template<typename F>
auto Task::functor_t<F>::move_to(storage_t& storage) -> functor_base_t*
{
    functor_base_t* r = nullptr;

    if constexpr (sizeof(f_) <= sizeof(storage_t)) {
        r = new (std::addressof(storage)) functor_t<std::decay_t<decltype(f_)>>{ std::move(f_) };
    }

    assert(r != nullptr);
    return r;
}
}

#endif // CYCLONITE_TASK_H
