//
// Created by bantdit on 12/31/19.
//

#include "eventReceivable.h"

namespace cyclonite {
std::atomic_uint_fast64_t EventReceivable::lastId_{ 0 };

EventReceivable::EventReceiver::EventReceiver(EventReceivable* instance)
  : instance_{ instance }
{
}

EventReceivable::EventReceivable()
  : id_{ ++lastId_ }
  , eventReceiver_{ std::make_shared<EventReceiver>(this) }
{
}

EventReceivable::EventReceivable(EventReceivable&& eventReceivable) noexcept
  : id_{ eventReceivable.id_ }
  , eventReceiver_{ std::move(eventReceivable.eventReceiver_) }
{
    eventReceivable.id_ = 0;
    eventReceiver_->instance(this);
}

EventReceivable& EventReceivable::operator=(EventReceivable&& rhs) noexcept
{
    eventReceiver_ = std::move(rhs.eventReceiver_);
    eventReceiver_->instance(this);

    id_ = rhs.id_;
    rhs.id_ = 0;

    return *this;
}
}
