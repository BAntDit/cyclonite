//
// Created by anton on 5/23/22.
//

#ifndef CYCLONITE_RESOURCE_H
#define CYCLONITE_RESOURCE_H

#include "resourceDependency.h"
#include <atomic>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <utility>

namespace cyclonite::resources {
class ResourceManager;

// non-copyable and non-movable
// can be accessed only by Id through the resource manager;
class Resource
{
    friend class ResourceManager;

protected:
    Resource() noexcept;

public:
    class Id
    {
    public:
        Id() noexcept;

        explicit Id(uint64_t id) noexcept;

        Id(uint32_t index, uint32_t version) noexcept;

        explicit operator uint64_t() const { return id_; }

        auto operator<=>(Id const& rhs) const -> int32_t = default;

        [[nodiscard]] auto index() const -> uint32_t { return static_cast<uint32_t>(id_ & 0xffffffffUL); }

        [[nodiscard]] auto version() const -> uint32_t { return static_cast<uint32_t>(id_ >> 32); }

    private:
        uint64_t id_;
    };

    Resource(Resource const&) = delete;

    Resource(Resource&&) = delete;

    virtual ~Resource() = default;

    [[nodiscard]] auto id() const -> Resource::Id { return id_; }

    auto operator=(Resource const&) -> Resource& = delete;

    auto operator=(Resource &&) -> Resource& = delete;

    template<typename T>
    auto as() -> T& requires std::derived_from<T, Resource>;

    template<typename T>
    [[nodiscard]] auto as() const -> T const& requires std::derived_from<T, Resource>;

    template<typename T>
    [[nodiscard]] auto is() const -> bool requires std::derived_from<T, Resource>;

protected:
    struct ResourceTag
    {
        friend class ResourceManager;

        ResourceTag() noexcept;

        std::uint16_t index;
        bool isFixedSizePerItem;

    private:
        static std::atomic_uint16_t _lastTagIndex;
    };

    [[nodiscard]] virtual auto instance_tag() const -> ResourceTag const& = 0;

private:
    static ResourceTag tag;
    static auto type_tag_const() -> ResourceTag const& { return tag; }
    static auto type_tag() -> ResourceTag& { return tag; }

private:
    using resource_dependencies_t = std::pair<ResourceDependency*, size_t>;

    Id id_;
    ResourceState state_;
    resource_dependencies_t dependencies_;
    ResourceManager* resourceManager_;
};

template<typename T>
auto Resource::as() const -> T const& requires std::derived_from<T, Resource>
{
    assert(instance_tag().index == T::type_tag_const().index);
    return static_cast<T>(*this);
}

template<typename T>
auto Resource::as() -> T& requires std::derived_from<T, Resource>
{
    return const_cast<T&>(std::as_const(*this).template as<T>());
}

template<typename T>
auto Resource::is() const -> bool requires std::derived_from<T, Resource>
{
    return instance_tag().index == T::type_tag_const().index;
}
}

#endif // CYCLONITE_RESOURCE_H
