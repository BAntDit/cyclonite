//
// Created by bantdit on 12/31/19.
//

#ifndef CYCLONITE_EVENTRECEIVABLE_H
#define CYCLONITE_EVENTRECEIVABLE_H

#include <atomic>
#include <memory>

namespace cyclonite {
class EventReceivable
{
public:
    class EventReceiver
    {
    public:
        explicit EventReceiver(EventReceivable* instance);

        ~EventReceiver() = default;

    public:
        [[nodiscard]] auto instance() const -> EventReceivable* { return instance_; }

        void instance(EventReceivable* instance) { instance_ = instance; }

    private:
        EventReceivable* instance_;
    };

public:
    EventReceivable(EventReceivable const& eventReceivable) = delete;

public:
    EventReceivable& operator=(EventReceivable const& rhs) = delete;

public:
    [[nodiscard]] auto eventReceiver() const -> std::shared_ptr<EventReceiver> const& { return eventReceiver_; }

    [[nodiscard]] auto id() const -> uint_fast64_t { return id_; }

protected:
    EventReceivable();

    EventReceivable(EventReceivable&& eventReceivable) noexcept;

    ~EventReceivable() = default;

protected:
    EventReceivable& operator=(EventReceivable&& rhs) noexcept;

private:
    std::uint_fast64_t id_;
    std::shared_ptr<EventReceiver> eventReceiver_;

private:
    static std::atomic_uint_fast64_t lastId_;
};
}

#endif // CYCLONITE_EVENTRECEIVABLE_H
