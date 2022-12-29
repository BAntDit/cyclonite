//
// Created by bantdit on 10/28/22.
//

#include "task.h"

namespace cyclonite::multithreading {
Task::Task()
  : storage_{}
  , functor_{ nullptr }
  , pending_{ false }
{}

Task::Task(Task&& task) noexcept
  : storage_{}
  , functor_{ nullptr }
  , pending_{ false }
{
    assert(task.functor_);

    if (task.functor_ == task.storage()) {
        functor_ = task.functor_->move_to(storage_);
        std::exchange(task.functor_, nullptr)->~functor_base_t();
    } else {
        functor_ = std::exchange(task.functor_, nullptr);
    }

    pending_.store(functor_ != nullptr, std::memory_order_release);
    task.pending_.store(false, std::memory_order_release);
}

auto Task::operator=(Task&& rhs) noexcept -> Task&
{
    assert(rhs.functor_);

    _reset();

    if (rhs.functor_ == rhs.storage()) {
        functor_ = rhs.functor_->move_to(storage_);
        rhs.functor_ = nullptr;
    } else {
        functor_ = std::exchange(rhs.functor_, nullptr);
    }

    pending_.store(functor_ != nullptr, std::memory_order_release);
    rhs.pending_.store(false, std::memory_order_release);

    return *this;
}

void Task::_reset()
{
    if (functor_ == storage()) {
        functor_->~functor_base_t();
    } else {
        delete functor_;
    }

    functor_ = nullptr;
}

void Task::operator()()
{
    assert(functor_);
    functor_->invoke();
}

Task::~Task()
{
    _reset();
}
}