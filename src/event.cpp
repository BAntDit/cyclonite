//
// Created by anton on 2/26/26.
//

#include "event.h"

namespace cyclonite {
EventReceivable::EventReceivable()
  : receiver_{ std::make_shared<EventReceiver>(this) }
{
}

EventReceivable::EventReceivable(EventReceivable const&)
  : EventReceivable{} // just creates new receiver on copy
{
}

EventReceivable::EventReceivable(EventReceivable&& eventReceivable) noexcept
  : receiver_{ std::move(eventReceivable.receiver_) }
{
    receiver_->instance(this);
}

auto EventReceivable::operator=(EventReceivable&& rhs) noexcept -> EventReceivable&
{
    receiver_ = std::move(rhs.receiver_);
    receiver_->instance(this);

    return *this;
}
}
