//
// Created by anton on 6/14/25.
//

#ifndef GFX_RESOURCE_REF_H
#define GFX_RESOURCE_REF_H

#include "resourceManager.h"
#include <limits>

namespace cyclonite::gfx {
class ResourceRef
{
    struct Id
    {
        explicit Id(uint64_t id) noexcept
          : id_{ id }
        {
        }

        Id() noexcept
          : id_{ std::numeric_limits<uint64_t>::max() }
        {
        }

        Id(uint32_t index, uint32_t version) noexcept
          : id_{ static_cast<uint64_t>(index) | static_cast<uint64_t>(version) << 32UL }
        {
        }

        explicit operator uint64_t() const { return id_; }

        bool operator==(Id rhs) const { return id_ == rhs.id_; }
        bool operator!=(Id rhs) const { return id_ != rhs.id_; }
        bool operator<(Id rhs) const { return id_ < rhs.id_; }

        [[nodiscard]] auto index() const -> uint32_t { return static_cast<uint32_t>(id_ & 0xffffff00UL); }

        [[nodiscard]] auto version() const -> uint32_t { return static_cast<uint8_t>((id_ >> 32UL) & 0xffffffffUL); }

    private:
        uint64_t id_;
    };

public:
    friend class ResourceManager;

    [[nodiscard]] auto id() const -> uint64_t { return id_; }

    [[nodiscard]] auto valid() const -> bool;

    template<typename T>
    [[nodiscard]] auto as() const -> T const&
        requires(resource_type_list_t::has_type<T>::value);

    template<typename T>
    [[nodiscard]] auto as() -> T&
        requires(resource_type_list_t::has_type<T>::value);

    auto retain() -> uint64_t;

    auto release() -> uint64_t;

    [[nodiscard]] auto refCount() const -> uint64_t;

private:
    explicit ResourceRef(ResourceManager* resourceManager);

    Id id_;
    ResourceManager* resourceManager_;
};
}

#endif // GFX_RESOURCE_REF_H
