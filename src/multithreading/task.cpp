//
// Created by bantdit on 10/28/22.
//

#include "task.h"

namespace cyclonite::multithreading
{
Task::Task(Task const& task)
  : storage_{}
  , functor_{ task.functor_ ? task.functor_->clone(storage_) : nullptr }
{}

Task::Task(Task&& task) noexcept
  : storage_{}
  , functor_{ nullptr }
{
    if (task.functor_ == task.storage()) {
        functor_ = task.functor_->move_to(storage_);
        std::exchange(task.functor_, nullptr)->~functor_base_t();
    } else {
        functor_ = std::exchange(task.functor_, nullptr);
    }
}

auto Task::operator=(Task const& rhs) -> Task&
{
    _reset();

    functor_ = rhs.functor_->clone(storage_);
    return *this;
}

auto Task::operator=(Task&& rhs) noexcept -> Task&
{
    _reset();

    if (rhs.functor_ == rhs.storage()) {
        functor_ = rhs.functor_->clone(storage_);
        rhs.functor_ = nullptr;
    } else {
        functor_ = std::exchange(rhs.functor_, nullptr);
    }

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

int Task::operator()()
{
    assert(functor_);
    functor_->invoke();
}

Task::~Task()
{
    _reset();
}
}