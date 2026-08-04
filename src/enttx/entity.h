//
// Created by anton on 8/4/26.
//

#ifndef CYCLONITE_ENTITY_H
#define CYCLONITE_ENTITY_H

#include <cstdint>
#include <limits>

#if defined(max)
#undef max
#endif

namespace cyclonite::enttx
{
class Entity
{
public:
    Entity() noexcept
      : id_{ std::numeric_limits<uint64_t>::max() }
    {
    }

    explicit Entity(uint64_t id)
      : id_{ id }
    {
    }

    Entity(uint32_t index, uint32_t version)
      : id_{ static_cast<uint64_t>(index) | static_cast<uint64_t>(version) << 32UL }
    {
    }

    explicit operator uint64_t() const { return id_; }

    bool operator==(Entity rhs) const { return id_ == rhs.id_; }
    bool operator!=(Entity rhs) const { return id_ != rhs.id_; }
    bool operator<(Entity rhs) const { return id_ < rhs.id_; }

    [[nodiscard]] auto index() const -> uint32_t { return static_cast<uint32_t>(id_ & 0xffffffffUL); }

    [[nodiscard]] auto version() const -> uint32_t { return static_cast<uint32_t>(id_ >> 32UL); }

private:
    uint64_t id_;
};
}

#endif //CYCLONITE_ENTITY_H