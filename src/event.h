//
// Created by bantdit on 12/31/19.
//

#ifndef CYCLONITE_EVENT_H
#define CYCLONITE_EVENT_H

#include "eventReceivable.h"
#include <algorithm>
#include <boost/functional/hash.hpp>
#include <functional>
#include <unordered_map>
#include <utility>
#include <variant>

namespace cyclonite {
template<typename... Args>
class Event
{
private:
    struct Dummy
    {
        void func(){};
        virtual void vFunc(){};
    };

    using event_handler_identifier_t =
      std::array<std::byte, sizeof(uint_fast64_t) + std::max(sizeof(&Dummy::func), sizeof(&Dummy::vFunc))>;

public:
    class EventHandler
    {
        friend Event;

    public:
        template<class T, class M>
        EventHandler(T* instance, void (M::*member)(Args...))
          : identifier_{}
          , handler_{}
        {
            auto id = instance->id();

            std::fill(identifier_.begin(), identifier_.end(), 0);

            std::copy(reinterpret_cast<std::byte*>(&id),
                      reinterpret_cast<std::byte*>(&id) + sizeof(uint_fast64_t),
                      identifier_.begin());

            std::copy(reinterpret_cast<std::byte*>(&member),
                      reinterpret_cast<std::byte*>(&member) + sizeof(member),
                      identifier_.begin() + sizeof(uint_fast64_t));

            auto eventReceiver = std::weak_ptr<EventReceivable::EventReceiver>{ instance->eventReceiver() };

            handler_ = [eventReceiver, member](auto&&... args) -> bool {
                if (auto receiver = eventReceiver.lock()) {
                    ((static_cast<T*>(receiver->instance()))->*member)(std::forward<decltype(args)>(args)...);
                    return true;
                }
                return false;
            };
        }

        explicit EventHandler(void (*handler)(Args...))
          : identifier_{}
          , handler_{}
        {
            std::fill(identifier_.begin(), identifier_.end(), 0);

            std::copy(reinterpret_cast<std::byte*>(handler),
                      reinterpret_cast<std::byte*>(handler) + sizeof(handler),
                      identifier_.begin() + sizeof(uint_fast64_t));

            handler_ = [handler](auto&&... args) -> bool {
                handler(std::forward<decltype(args)>(args)...);
                return true;
            };
        }

        EventHandler(EventHandler const&) = default;

        EventHandler(EventHandler&&) = default;

        ~EventHandler() = default;

    public:
        EventHandler& operator=(EventHandler const&) = default;

        EventHandler& operator=(EventHandler&&) = default;

        template<typename... EventArgs>
        bool operator()(EventArgs&&... args)
        {
            return invoke(std::forward<EventArgs>(args)...);
        }

    public:
        template<typename... EventArgs>
        bool invoke(EventArgs&&... args)
        {
            return handler_(std::forward<EventArgs>(args)...);
        }

    public:
        auto identifier() const -> event_handler_identifier_t const& { return identifier_; }

    private:
        event_handler_identifier_t identifier_;
        std::function<bool(Args...)> handler_;
    };

public:
    Event()
      : eventHandlers_{}
    {}

    Event(Event const&) = delete;

    Event(Event&&) = default;

    ~Event() = default;

public:
    Event& operator=(Event const&) = delete;

    Event& operator=(Event&&) = default;

    template<typename Handler>
    auto operator+=(Handler const& rhs) -> EventHandler
    {
        static_assert(std::is_same_v<EventHandler, std::decay_t<Handler>> || std::is_invocable_v<Handler, Args...>);

        if constexpr (std::is_same_v<EventHandler, std::decay_t<Handler>>) {
            eventHandlers_.emplace(rhs.identifier(), rhs);
            return rhs;
        } else {
            EventHandler eh{ rhs };
            eventHandlers_.emplace(eh.identifier(), eh);
            return eh;
        }
    }

    void operator-=(EventHandler const& rhs) { eventHandlers_.erase(rhs.identifier()); }

    template<typename... EventArgs>
    void operator()(EventArgs&&... args)
    {
        auto it = eventHandlers_.begin();

        while (it != eventHandlers_.end()) {
            EventHandler& eventHandler = (*it).second;

            if (eventHandler(std::forward<EventArgs>(args)...)) {
                it++;
            } else {
                it = eventHandlers_.erase(it);
            }
        }
    }

private:
    std::unordered_map<event_handler_identifier_t, EventHandler, boost::hash<event_handler_identifier_t>>
      eventHandlers_;
};
}

#endif // CYCLONITE_EVENT_H
