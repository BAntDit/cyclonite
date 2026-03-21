//
// Created by anton on 3/8/26.
//

#ifndef CYCLONITE_RESOURCES_MANAGED_RESOURCE_H
#define CYCLONITE_RESOURCES_MANAGED_RESOURCE_H

#include "core/resourceBase.h"
#include "managedResourceState.h"
#include "multithreading/taskManager.h"
#include <atomic>
#include <concepts>
#include <future>
#include <metrix/type_traits.h>
#include <stdexcept>
#include <type_traits>

namespace cyclonite::resources {
template<typename T>
concept is_loadable = requires(T t) {
    { t.loadImpl() } -> std::same_as<std::future<void>>;
};

/*
template<typename T>
concept is_initializeable = requires(T t) {
requires std::is_member_function_pointer_v<decltype(&T::initialize)> &&
           std::is_same_v<bool, metrix::member_function_return_type_t<decltype(&T::initialize)>>;
};

template<typename T>
concept is_resetable = requires(T t) {
{ t.reset() } -> std::same_as<void>;
};*/

template<typename T>
concept ManagedResourceConcept = std::is_base_of_v<core::ResourceBase, T>;

template<typename Resource>
class ManagedResource
{
public:
    ManagedResource()
      : state_{ ManagedResourceState::Initial }
      , loadingResult_{}
    {
    }

    auto load() -> std::shared_future<void>;

    // void reset();

    [[nodiscard]] auto state() const -> ManagedResourceState { return state_.load(std::memory_order_acquire); }

private:
    void setState(ManagedResourceState state);

    std::atomic<ManagedResourceState> state_;
    std::shared_future<void> loadingResult_;
};

template<typename Resource>
auto ManagedResource<Resource>::load() -> std::shared_future<void>
{
    auto expectedState = ManagedResourceState::Initial;
    if (state_.compare_exchange_weak(
          expectedState, ManagedResourceState::Loading, std::memory_order_release, std::memory_order_relaxed)) {
        if constexpr (is_loadable<Resource>) {
            // TODO::
        } else {
            auto promise = std::promise<void>();
            auto future = promise.get_future();
            promise.set_value();
            setState(ManagedResourceState::Loaded);
            loadingResult_ = std::move(future);
        }
    }

    return loadingResult_;
}

template<typename Resource>
void ManagedResource<Resource>::setState(ManagedResourceState state)
{
    if (state == ManagedResourceState::Loaded) {
        auto expectedState = ManagedResourceState::Loading;
        if (!state_.compare_exchange_weak(
              expectedState, ManagedResourceState::Loaded, std::memory_order_release, std::memory_order_relaxed)) {
            throw std::logic_error("load state can be set from loading state only");
        }
    } else {
        throw std::logic_error("attempt to set wrong resource state");
    }
}

/*
template<typename Resource>
void ManagedResource<Resource>::reset()
{
    auto expectedState = LoadingState::Ready;
    while (!loadingState_.compare_exchange_weak(
      expectedState, LoadingState::Reseting, std::memory_order_acq_rel, std::memory_order_acquire)) {
        if (expectedState != LoadingState::Ready && expectedState != LoadingState::Loaded &&
            expectedState != LoadingState::RawPartUnloadedReady && expectedState != LoadingState::Unloaded) {
            throw std::logic_error("could not reset resource from intermediate state");
        }
    }

    if constexpr (is_resetable<Resource>) {
        static_cast<Resource*>(this)->reset();
    }

    setState(LoadingState::Initial);
}

template<typename Resource>
auto ManagedResource<Resource>::load() -> std::future<bool>
{
    auto result = false;
    auto expectedState = LoadingState::Initial;
    if (loadingState_.compare_exchange_weak(
          expectedState, LoadingState::Loading, std::memory_order_release, std::memory_order_relaxed)) {
        if constexpr (is_loadable<Resource>) {
            auto* res = static_cast<Resource*>(this);
            auto resId = res->resourceId();
            auto& managerBase = res->resourceBase()->resourceManager();

            return multithreading::Executor::threadExecutor().taskManager().submitTask(
              [f = static_cast<Resource*>(this)->load(), resource = this, resId, &managerBase]() -> bool {
                  if (auto r = f.get(); r) {
                      if (managerBase.isResourceValid(resId)) {
                          resource->setState(LoadingState::Loaded);
                          return true;
                      }
                  }

                  if (managerBase.isResourceValid(resId)) {
                      resource->setState(LoadingState::Corrupted);
                      return true;
                  }

                  return false;
              });
        } else {
            setState(LoadingState::Loaded);
            result = true;
        }
    } else {
        throw std::logic_error("loading can be start from initial state only");
    }

    auto promise = std::promise<bool>();
    auto future = promise.get_future();

    promise.set_value(result);

    return future;
}

template<typename Resource>
void ManagedResource<Resource>::setState(LoadingState state)
{
    if (state == LoadingState::Undefined) {
        throw std::logic_error("attempt to set wrong resource state");
    }

    if (state == LoadingState::Loaded) {
        auto expectedState = LoadingState::Loading;
        if (!loadingState_.compare_exchange_weak(
              expectedState, LoadingState::Loaded, std::memory_order_release, std::memory_order_relaxed)) {
            throw std::logic_error("attempt to set wrong resource state");
        }
    } else if (state == LoadingState::Initial) {
        auto expectedState = LoadingState::Undefined;
        while (!loadingState_.compare_exchange_weak(
          expectedState, LoadingState::Initial, std::memory_order_acq_rel, std::memory_order_acquire)) {
            if (expectedState != LoadingState::Undefined && expectedState != LoadingState::Reseting) {
                throw std::logic_error("attempt to set wrong resource state");
            }
        }
    } else {
        throw std::logic_error("attempt to set wrong resource state");
    }

    // TODO:: other states
}*/
}

#endif // CYCLONITE_RESOURCES_MANAGED_RESOURCE_H