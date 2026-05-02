//
// Created by anton on 3/8/26.
//

#ifndef CYCLONITE_RESOURCES_MANAGED_RESOURCE_H
#define CYCLONITE_RESOURCES_MANAGED_RESOURCE_H

#include "core/resourceSharedRef.h"
#include "core/resourceWeakRef.h"
#include "managedResourceState.h"
#include "multithreading/taskManager.h"
#include "resourceGroupBase.h"
#include <atomic>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/uuid/uuid.hpp>
#include <concepts>
#include <filesystem>
#include <fstream>
#include <future>
#include <stdexcept>
#include <type_traits>
#include <variant>

namespace cyclonite::resources {
template<typename T>
concept is_loadable = requires(T t, std::istream& stream) {
    { t.loadImpl(stream) } -> std::same_as<void>;
};

template<typename T>
concept is_prepareable = requires(T t) {
    { t.prepareImpl() } -> std::same_as<void>;
};

template<typename Resource>
class ManagedResource
{
    struct loading_context_t
    {
        loading_context_t(std::filesystem::path const& path, std::ios_base::openmode mode);

        loading_context_t(std::byte const* data, size_t size);

        explicit loading_context_t(std::istream& stream);

        loading_context_t(loading_context_t&&) = default;

        loading_context_t(loading_context_t const&) = delete;

        ~loading_context_t();

        auto operator=(loading_context_t&&) -> loading_context_t& = default;

        auto operator=(loading_context_t const&) -> loading_context_t& = delete;

        [[nodiscard]] auto stream() -> std::istream&;

    private:
        std::variant<std::monostate,
                     std::unique_ptr<std::ifstream>,
                     std::unique_ptr<boost::iostreams::stream<boost::iostreams::array_source>>,
                     std::istream*>
          source_;
    };

public:
    ManagedResource(ResourceGroupBase* ownerGroup, std::string_view name, boost::uuids::uuid const& uuid)
      : ownerGroup_{ ownerGroup }
      , state_{ ManagedResourceState::Initial }
      , loadingResult_{}
      , name_(name)
      , uuid_(uuid)
    {
        assert(ownerGroup_ != nullptr);
        ownerGroup_->notifyResourceAdded(name);
    }

    auto load(std::filesystem::path path, std::ios_base::openmode mode) -> std::shared_future<void>;

    auto load(std::istream& stream) -> std::shared_future<void>;

    auto load(std::byte const* data, size_t size) -> std::shared_future<void>;

    auto prepare() -> std::shared_future<void>;

    // void reset();

    [[nodiscard]] auto state() const -> ManagedResourceState { return state_.load(std::memory_order_acquire); }

    [[nodiscard]] auto uuid() const -> boost::uuids::uuid const& { return uuid_; }

    [[nodiscard]] auto name() const -> std::string_view { return name_; }

    [[nodiscard]] auto managedResource() const -> ManagedResource<Resource> const& { return *this; }

    [[nodiscard]] auto managedResource() -> ManagedResource<Resource>& { return *this; }

    [[nodiscard]] auto group() const -> ResourceGroupBase const& { return *ownerGroup_; }

    [[nodiscard]] auto group() -> ResourceGroupBase& { return *ownerGroup_; }

private:
    auto loadInternal(loading_context_t&& loadingContext) -> std::shared_future<void>;

    void setState(ManagedResourceState state);

    ResourceGroupBase* ownerGroup_;
    std::atomic<ManagedResourceState> state_;
    std::shared_future<void> loadingResult_;
    std::shared_future<void> preparationResult_;
    std::string name_;
    boost::uuids::uuid uuid_;
};

template<typename Resource>
ManagedResource<Resource>::loading_context_t::loading_context_t(std::istream& stream)
  : source_{ &stream }
{
}

template<typename Resource>
ManagedResource<Resource>::loading_context_t::loading_context_t(std::byte const* data, size_t size)
  : source_{ std::make_unique<boost::iostreams::stream<boost::iostreams::array_source>>(
      reinterpret_cast<char const*>(data),
      size) }
{
}

template<typename Resource>
ManagedResource<Resource>::loading_context_t::loading_context_t(std::filesystem::path const& path,
                                                                std::ios_base::openmode mode)
  : source_{}
{
    auto file = std::make_unique<std::ifstream>();

    file->exceptions(std::ios::failbit);
    file->open(path.string(), mode);
    file->exceptions(std::ios::badbit);

    source_ = std::move(file);
}

template<typename Resource>
ManagedResource<Resource>::loading_context_t::~loading_context_t()
{
    if (source_.index() == 1) {
        auto& stream = std::get<std::unique_ptr<std::ifstream>>(source_);
        stream->close();
    }
}

template<typename Resource>
[[nodiscard]] auto ManagedResource<Resource>::loading_context_t::stream() -> std::istream&
{
    return std::visit(
      [](auto&& s) -> std::istream& {
          if constexpr (!std::is_same_v<std::decay_t<decltype(s)>, std::monostate>) {
              return *s;
          }
          throw std::runtime_error("invalid loading context");
      },
      source_);
}

template<typename Resource>
auto ManagedResource<Resource>::load(std::filesystem::path path,
                                     std::ios_base::openmode mode) -> std::shared_future<void>
{
    return loadInternal(loading_context_t{ path, mode });
}

template<typename Resource>
auto ManagedResource<Resource>::load(std::istream& stream) -> std::shared_future<void>
{
    return loadInternal(loading_context_t{ stream });
}

template<typename Resource>
auto ManagedResource<Resource>::load(std::byte const* data, size_t size) -> std::shared_future<void>
{
    return loadInternal(loading_context_t{ data, size });
}

template<typename Resource>
auto ManagedResource<Resource>::loadInternal(loading_context_t&& loadingContext) -> std::shared_future<void>
{
    auto expectedState = ManagedResourceState::Initial;
    if (state_.compare_exchange_weak(
          expectedState, ManagedResourceState::Loading, std::memory_order_release, std::memory_order_relaxed)) {
        ownerGroup_->notifyResourceStateChange(ManagedResourceState::Loading, name_);

        if constexpr (is_loadable<Resource>) {
            auto* res = static_cast<Resource*>(this);
            auto weakRef = core::ResourceWeakRef{ core::makeResourceSharedRefUnsafe(res) };

            loadingResult_ = multithreading::Executor::threadExecutor().taskManager().submitTask(
              [weakRef, loadingContext = std::move(loadingContext)]() mutable -> void {
                  if (auto sharedRef = weakRef.lock(); sharedRef.valid()) {
                      auto& r = sharedRef.as<Resource>();
                      r.loadImpl(loadingContext.stream());
                      r.setState(ManagedResourceState::Loaded);
                  }
              });
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
auto ManagedResource<Resource>::prepare() -> std::shared_future<void>
{
    auto expectedState = is_loadable<Resource> ? ManagedResourceState::Loaded : ManagedResourceState::Initial;

    if (state_.compare_exchange_weak(
          expectedState, ManagedResourceState::Preparing, std::memory_order_release, std::memory_order_relaxed)) {
        ownerGroup_->notifyResourceStateChange(ManagedResourceState::Preparing, name_);

        if constexpr (is_prepareable<Resource>) {
            auto* res = static_cast<Resource*>(this);
            auto weakRef = core::ResourceWeakRef{ core::makeResourceSharedRefUnsafe(res) };

            preparationResult_ =
              multithreading::Executor::threadExecutor().taskManager().submitTask([weakRef]() mutable -> void {
                  if (auto sharedRef = weakRef.lock(); sharedRef.valid()) {
                      auto& r = sharedRef.as<Resource>();
                      r.prepareImpl();
                      r.setState(ManagedResourceState::Ready);
                  }
              });
        } else {
            auto promise = std::promise<void>();
            auto future = promise.get_future();
            promise.set_value();
            setState(ManagedResourceState::Ready);
            preparationResult_ = std::move(future);
        }
    }

    return preparationResult_;
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
        ownerGroup_->notifyResourceStateChange(ManagedResourceState::Loaded, name_);
    } else if (state == ManagedResourceState::GoingToBeRemoved) {
        state_.store(ManagedResourceState::GoingToBeRemoved, std::memory_order_release);
        ownerGroup_->notifyResourceStateChange(ManagedResourceState::GoingToBeRemoved, name_);
    } else if (state == ManagedResourceState::Ready) {
        auto expectedState = ManagedResourceState::Preparing;
        if (!state_.compare_exchange_weak(
              expectedState, ManagedResourceState::Loaded, std::memory_order_release, std::memory_order_relaxed)) {
            throw std::logic_error("load state can be set from loading state only");
        }
        ownerGroup_->notifyResourceStateChange(ManagedResourceState::Ready, name_);
    } else {
        throw std::logic_error("attempt to set wrong resource state");
    }
}
}

#endif // CYCLONITE_RESOURCES_MANAGED_RESOURCE_H