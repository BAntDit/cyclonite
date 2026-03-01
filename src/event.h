//
// Created by bantdit on 12/31/19.
//

#ifndef CYCLONITE_EVENT_H
#define CYCLONITE_EVENT_H

#include <any>
#include <cassert>
#include <memory>
#include <metrix/type_traits.h>
#include <type_traits>
#include <utility>
#include <list>

namespace cyclonite {
class EventReceivable
{
public:
    class EventReceiver
    {
        friend class EventReceivable;

    public:
        EventReceiver() = default;

        explicit EventReceiver(EventReceivable* eventReceivable)
          : instance_{ eventReceivable }
        {
        }

        [[nodiscard]] auto instance() const -> EventReceivable* { return instance_; }

    private:
        void instance(EventReceivable* receivable) { instance_ = receivable; }

        EventReceivable* instance_;
    };

    EventReceivable(EventReceivable const&);

    EventReceivable(EventReceivable&& eventReceivable) noexcept;

    auto operator=(EventReceivable const&) -> EventReceivable& { return *this; } // don't copy receiver

    auto operator=(EventReceivable&& rhs) noexcept -> EventReceivable&;

    [[nodiscard]] auto receiver() const -> std::shared_ptr<EventReceiver> { return receiver_; }

protected:
    EventReceivable();

private:
    std::shared_ptr<EventReceiver> receiver_;
};

template<typename T>
concept EventReceivableConcept = std::derived_from<T, EventReceivable>;

namespace internal {
template<typename MemberFunctionPtr>
struct deferred_invoke_t
{
    template<typename Instance, typename... Args>
        requires std::is_same_v<metrix::member_function_argument_type_list_t<MemberFunctionPtr>,
                                metrix::type_list<Args...>> &&
                 std::is_same_v<metrix::member_function_class_type_t<MemberFunctionPtr>, Instance>
    void operator()(Instance* instance, Args... args)
    {
        instance->*member_(args...);
    }

private:
    MemberFunctionPtr member_;
};

template<typename... Args>
struct invoker_t
{
    template<EventReceivableConcept Instance, typename DeferredInvoke>
    invoker_t(Instance* instance, DeferredInvoke&& deferredInvoke)
      : instance_{ instance->receiver() }
      , deferred_{ std::forward<DeferredInvoke>(deferredInvoke) }
      , f_{ &memberFunctionInvoke<Instance, std::decay_t<DeferredInvoke>> }
      , isFreeFunction_{ false }
    {
    }

    template<typename FreeFunctionPtr>
    explicit invoker_t(FreeFunctionPtr freeFunctionPtr)
      : instance_{}
      , deferred_{ freeFunctionPtr }
      , f_{ &freeFunctionInvoke<FreeFunctionPtr> }
      , isFreeFunction_{ true }
    {
    }

    invoker_t(invoker_t const&) = delete;

    invoker_t(invoker_t&& other) noexcept
      : instance_{ std::exchange(other.instance_, std::weak_ptr<EventReceivable::EventReceiver>{}) }
      , deferred_{ std::exchange(other.deferred_, std::any{}) }
      , f_{ other.f_ }
      , isFreeFunction_{ other.isFreeFunction_ }
    {
    }

    auto operator=(invoker_t const&) -> invoker_t& = delete;

    auto operator=(invoker_t&& rhs) noexcept -> invoker_t&
    {
        instance_ = std::exchange(rhs.instance_, std::weak_ptr<EventReceivable::EventReceiver>{});
        deferred_ = std::exchange(rhs.deferred_, std::any{});
        f_ = rhs.f_;
        isFreeFunction_ = rhs.isFreeFunction_;

        return *this;
    }

    template<typename... Argument>
        requires(std::is_convertible_v<Argument, Args> && ...)
    auto operator()(Argument&&... args) -> bool
    {
        if (isFreeFunction()) {
            f_(nullptr, deferred_, std::forward<Argument>(args)...);
            return true;
        }

        if (auto receiver = instance_.lock()) {
            f_(receiver->instance(), deferred_, std::forward<Argument>(args)...);
            return true;
        }

        return false;
    }

    [[nodiscard]] auto isFreeFunction() const -> bool { return isFreeFunction_; }

private:
    template<typename FreeFunctionPtr>
    static void freeFunctionInvoke(void*, std::any const& def, Args... args)
    {
        assert(def.has_value());
        std::any_cast<FreeFunctionPtr>(def)(args...);
    }

    template<typename Instance, typename DeferredInvoke>
    static void memberFunctionInvoke(void* instance, std::any const& def, Args... args)
    {
        assert(instance != nullptr);
        assert(def.has_value());
        std::any_cast<DeferredInvoke>(def)(static_cast<Instance*>(instance), args...);
    }

    using invoke_f = void (*)(void*, std::any const& def, Args... args);

    std::weak_ptr<EventReceivable::EventReceiver> instance_;
    std::any deferred_;

    invoke_f f_;

    bool isFreeFunction_;
};
};

template <typename Handler>
class EventHandler;

template<typename... Args>
class Event
{
public:
    Event() = default;

    // do not copy handlers
    Event(Event const&) : Event() {}

    Event(Event&&) = default;

    auto operator=(Event const&) -> Event& { return *this; };

    auto operator=(Event&&) -> Event& = default;

    template<typename... Argument>
    void operator()(Argument&&... argument);

private:
    std::list<internal::invoker_t<Args...>> handlers_;
};

template<typename... Args>
template<typename... Argument>
void Event<Args...>::operator()(Argument&&... argument) {
    auto it = std::begin(handlers_);
    while (it != std::end(handlers_)) {
        if ((*it)(std::forward<Argument>(argument)...)) {
            it++;
        } else {
            it = handlers_.erase(it);
        }
    }
}

}

#endif // CYCLONITE_EVENT_H
