//
// Created by anton on 9/9/25.
//

#ifndef CYCLONITE_MULTITHREADING_TASK_H
#define CYCLONITE_MULTITHREADING_TASK_H

#include "common.h"
#include <cassert>
#include <atomic>

namespace cyclonite::multithreading
{
class Task;
class StrandTask;

template<typename T>
concept TaskFunctorConcept = !std::is_same_v<std::decay_t<T>, Task> && !std::is_same_v<std::decay_t<T>, StrandTask>;

namespace internal
{
inline constexpr size_t storage_align_v = alignof(void (*)());
inline constexpr size_t storage_size_v = 64;

struct functor_base_t
{
    functor_base_t() = default;

    virtual ~functor_base_t() = default;

    virtual void invoke() = 0;

    virtual auto move_to(std::byte (&storage)[storage_size_v]) -> functor_base_t* = 0;
};

template<typename F>
struct functor_t : functor_base_t
{
    explicit functor_t(F const& f)
            : f_(f)
    {
    }

    explicit functor_t(F&& f)
            : f_(std::move(f))
    {
    }

    void invoke() override { f_(); }

    auto move_to(std::byte (&storage)[storage_size_v]) -> functor_base_t* override;

private:
    F f_;
};

template<typename F>
auto functor_t<F>::move_to(std::byte (&storage)[storage_size_v]) -> functor_base_t*
{
    auto* r = std::add_pointer_t<functor_base_t>{ nullptr };

    void* sdata = std::data(storage);
    size_t ssize = std::size(storage);

    if (auto* p = std::align(alignof(functor_t<F>), sizeof(functor_t<F>), sdata, ssize); p == std::data(storage)) {
        r = new (p) functor_t<F>{ std::move(f_) };
    }

    assert(r != nullptr);
    return r;
}
}

class alignas(hardware_destructive_interference_size) Task
{
public:
    Task();

    template<TaskFunctorConcept F>
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

    auto storage() -> void* { return storage_; }

private:
    alignas(internal::storage_align_v) std::byte storage_[internal::storage_size_v]; // for small functor optimization (sfo)
    internal::functor_base_t* functor_;
    std::atomic<bool> pending_;
};

class StrandDeque;
}

#endif //CYCLONITE_MULTITHREADING__TASK_H
