//
// Created by bantdit on 12/31/19.
//

#ifndef CYCLONITE_EVENT_H
#define CYCLONITE_EVENT_H

#include <any>
#include <cassert>
#include <list>
#include <memory>
#include <metrix/type_traits.h>
#include <type_traits>
#include <utility>

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
struct member_function_deferred_invoke_t
{
    explicit member_function_deferred_invoke_t(MemberFunctionPtr member)
      : member_{ member }
    {
    }

    template<typename... Args>
        requires std::is_same_v<metrix::member_function_argument_type_list_t<MemberFunctionPtr>,
                                metrix::type_list<std::decay_t<Args>...>>
    void operator()(EventReceivable* instance, Args&&... args)
    {
        (static_cast<metrix::member_function_class_type_t<MemberFunctionPtr>*>(instance)->*member_)(
          std::forward<Args>(args)...);
    }

    static inline const std::byte id = std::byte{ 0 };
    static auto typeId() -> uintptr_t { return reinterpret_cast<uintptr_t>(&id); }

private:
    MemberFunctionPtr member_;
};

template<typename FreeFunctionPtr>
struct free_function_deferred_invoke_t
{
    explicit free_function_deferred_invoke_t(FreeFunctionPtr func)
      : func_{ func }
    {
    }

    template<typename... Args>
    void operator()(Args&&... args)
    {
        func_(std::forward<Args>(args)...);
    }

    static const std::byte id = std::byte{ 0 };
    static auto typeId() -> uintptr_t { return reinterpret_cast<uintptr_t>(&id); }

private:
    FreeFunctionPtr func_;
};

template<typename ArgList>
struct invoker_t;

template<typename... Args>
struct invoker_t<metrix::type_list<Args...>>
{
    template<EventReceivableConcept Instance, typename DeferredInvoke>
    invoker_t(Instance* instance, DeferredInvoke&& deferredInvoke)
      : instance_{ instance->receiver() }
      , deferred_{ std::forward<DeferredInvoke>(deferredInvoke) }
      , f_{ &memberFunctionInvoke<Instance, std::decay_t<DeferredInvoke>> }
      , invokeId_{ std::decay_t<DeferredInvoke>::typeId() }
      , isFreeFunction_{ false }
    {
    }

    template<typename DeferredInvoke>
    explicit invoker_t(DeferredInvoke deferredInvoke)
      : instance_{}
      , deferred_{ deferredInvoke }
      , f_{ &freeFunctionInvoke<std::decay_t<DeferredInvoke>> }
      , invokeId_{ std::decay_t<DeferredInvoke>::typeId() }
      , isFreeFunction_{ true }
    {
    }

    invoker_t(invoker_t const&) = delete;

    invoker_t(invoker_t&& other) noexcept
      : instance_{ std::exchange(other.instance_, std::weak_ptr<EventReceivable::EventReceiver>{}) }
      , deferred_{ std::exchange(other.deferred_, std::any{}) }
      , f_{ other.f_ }
      , invokeId_{ std::exchange(other.invokeId_, 0) }
      , isFreeFunction_{ other.isFreeFunction_ }
    {
    }

    auto operator=(invoker_t const&) -> invoker_t& = delete;

    auto operator=(invoker_t&& rhs) noexcept -> invoker_t&
    {
        instance_ = std::exchange(rhs.instance_, std::weak_ptr<EventReceivable::EventReceiver>{});
        deferred_ = std::exchange(rhs.deferred_, std::any{});
        f_ = rhs.f_;
        invokeId_ = std::exchange(rhs.invokeId_, 0);
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

    template<typename FreeFunctionPtr>
    [[nodiscard]] auto isEqual(FreeFunctionPtr) const -> bool
    {
        if (!isFreeFunction())
            return false;

        if (free_function_deferred_invoke_t<FreeFunctionPtr>::typeId() != invokeId_)
            return false;

        return true;
    }

    template<typename MemberFunctionPtr>
    [[nodiscard]] auto isEqual(EventReceivable* instance, MemberFunctionPtr) const -> bool
    {
        if (isFreeFunction())
            return false;

        if (auto receiver = instance_.lock()) {
            if (receiver->instance() != instance) {
                return false;
            }

            if (member_function_deferred_invoke_t<MemberFunctionPtr>::typeId() != invokeId_) {
                return false;
            }

            return true;
        }

        return false;
    }

    [[nodiscard]] auto isFreeFunction() const -> bool { return isFreeFunction_; }

private:
    template<typename DeferredInvoke>
    static void freeFunctionInvoke(void*, std::any const& def, Args... args)
    {
        assert(def.has_value());
        std::any_cast<DeferredInvoke>(def)(args...);
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
    uintptr_t invokeId_;
    bool isFreeFunction_;
};
};

template<typename... Args>
class Event;

template<typename Handler>
class EventHandler;

template<EventReceivableConcept Instance, typename MemberFunctionPtr>
class EventHandler<metrix::type_pair<Instance, MemberFunctionPtr>>
  : public internal::invoker_t<metrix::member_function_argument_type_list_t<MemberFunctionPtr>>
{
public:
    EventHandler(Instance* instance, MemberFunctionPtr memberFunctionPtr)
      : internal::invoker_t<
          metrix::member_function_argument_type_list_t<MemberFunctionPtr>>{ instance,
                                                                            internal::member_function_deferred_invoke_t{
                                                                              memberFunctionPtr } }
      , member_{ memberFunctionPtr }
      , instance_{ instance }
    {
    }

    [[nodiscard]] auto getInvoker()
      -> internal::invoker_t<metrix::member_function_argument_type_list_t<MemberFunctionPtr>>&
    {
        return *this;
    }

    template<typename... Args>
    [[nodiscard]] auto isEqual(internal::invoker_t<metrix::type_list<Args...>> const& invoker) const -> bool
    {
        return invoker.isEqual(instance_, member_);
    }

private:
    MemberFunctionPtr member_;
    EventReceivable* instance_;
};

template<EventReceivableConcept Instance, typename MemberFunctionPtr>
EventHandler(Instance* instance,
             MemberFunctionPtr memberFunctionPtr) -> EventHandler<metrix::type_pair<Instance, MemberFunctionPtr>>;

template<typename... Args>
class EventHandler<void (*)(Args...)> : public internal::invoker_t<metrix::type_list<Args...>>
{
public:
    explicit EventHandler(void (*handler)(Args...))
      : internal::invoker_t<metrix::type_list<Args...>>{ handler }
      , freeFuncHandler_{ handler }
    {
    }

    [[nodiscard]] auto getInvoker() -> internal::invoker_t<metrix::type_list<Args...>>& { return *this; }

    template<typename... Arguments>
    [[nodiscard]] auto isEqual(internal::invoker_t<metrix::type_list<Arguments...>> const& invoker) const -> bool
    {
        return invoker.isEqual(freeFuncHandler_);
    }

private:
    using handler_f = void (*)(Args...);
    handler_f freeFuncHandler_;
};

template<typename... Args>
class Event
{
public:
    Event() = default;

    // do not copy handlers
    Event(Event const&)
      : Event()
    {
    }

    Event(Event&&) = default;

    auto operator=(Event const&) -> Event& { return *this; };

    auto operator=(Event&&) -> Event& = default;

    template<typename... Argument>
    void operator()(Argument&&... argument)
        requires(std::is_convertible_v<Argument, Args> && ...);

    template<typename Handler>
    void operator+=(EventHandler<Handler>&& handler);

    template<typename Handler>
    void operator-=(EventHandler<Handler>&& handler);

private:
    std::list<internal::invoker_t<metrix::type_list<Args...>>> handlers_;
};

template<typename... Args>
template<typename Handler>
void Event<Args...>::operator+=(EventHandler<Handler>&& handler)
{
    handlers_.push_back(std::move(handler.getInvoker()));
}

template<typename... Args>
template<typename Handler>
void Event<Args...>::operator-=(EventHandler<Handler>&& handler)
{
    auto it = std::begin(handlers_);
    while (it != std::end(handlers_)) {
        if (handler.isEqual(*it)) {
            it = handlers_.erase(it);
            break;
        } else {
            it++;
        }
    }
}

template<typename... Args>
template<typename... Argument>
void Event<Args...>::operator()(Argument&&... argument)
    requires(std::is_convertible_v<Argument, Args> && ...)
{
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
