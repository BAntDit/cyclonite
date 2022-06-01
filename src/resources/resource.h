//
// Created by anton on 5/23/22.
//

#ifndef CYCLONITE_RESOURCE_H
#define CYCLONITE_RESOURCE_H

#include "resourceState.h"
#include <atomic>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <filesystem>
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

    explicit Resource(size_t dynamicSize) noexcept;

    [[nodiscard]] auto dynamicDataSize() const -> size_t { return dynamicSize_; }

    [[nodiscard]] auto dynamicDataOffset() const -> size_t { return dynamicOffset_; }

    auto dynamicData() -> std::byte*;

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

    virtual void load(std::filesystem::path const& path);

    virtual void load(void const* data, size_t size);

    virtual void load(std::istream& stream);

protected:
    struct ResourceTag
    {
        friend class ResourceManager;

        ResourceTag() noexcept;

        uint16_t staticDataIndex;  // index to the data which size is known in compile time (type itself)
        uint16_t dynamicDataIndex; // index to the data which size is known in runtime only (dynamically allocated)

    private:
        static std::atomic_uint16_t _lastTagIndex;
    };

    [[nodiscard]] virtual auto instance_tag() const -> ResourceTag const& = 0;

private:
    static ResourceTag tag;
    static auto type_tag_const() -> ResourceTag const& { return tag; }
    static auto type_tag() -> ResourceTag& { return tag; }

private:
    Id id_;
    ResourceManager* resourceManager_;
    size_t dynamicOffset_;
    size_t dynamicSize_;

protected:
    std::atomic<ResourceState> state_;
};

template<typename T>
auto Resource::as() const -> T const& requires std::derived_from<T, Resource>
{
    assert(instance_tag().staticDataIndex == T::type_tag_const().staticDataIndex);
    return *static_cast<T const*>(this);
}

template<typename T>
auto Resource::as() -> T& requires std::derived_from<T, Resource>
{
    return const_cast<T&>(std::as_const(*this).template as<T>());
}

template<typename T>
auto Resource::is() const -> bool requires std::derived_from<T, Resource>
{
    return instance_tag().staticDataIndex == T::type_tag_const().staticDataIndex;
}
}

#endif // CYCLONITE_RESOURCE_H
