//
// Created by anton on 6/15/25.
//

#ifndef GFX_RESOURCE_ID_H
#define GFX_RESOURCE_ID_H

#include <cstdint>
#include <limits>

namespace cyclonite::gfx {
struct ResourceId
{
    explicit ResourceId(uint64_t id) noexcept
      : id_{ id }
    {
    }

    ResourceId() noexcept
      : id_{ std::numeric_limits<uint64_t>::max() }
    {
    }

    ResourceId(uint32_t index, uint32_t version) noexcept
      : id_{ static_cast<uint64_t>(index) | static_cast<uint64_t>(version) << 32UL }
    {
    }

    explicit operator uint64_t() const { return id_; }

    bool operator==(ResourceId rhs) const { return id_ == rhs.id_; }
    bool operator!=(ResourceId rhs) const { return id_ != rhs.id_; }
    bool operator<(ResourceId rhs) const { return id_ < rhs.id_; }

    [[nodiscard]] auto index() const -> uint32_t { return static_cast<uint32_t>(id_ & 0xffffff00UL); }

    [[nodiscard]] auto version() const -> uint32_t { return static_cast<uint8_t>((id_ >> 32UL) & 0xffffffffUL); }

private:
    uint64_t id_;
};
}

#endif // GFX_RESOURCE_ID_H
