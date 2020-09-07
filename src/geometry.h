//
// Created by bantdit on 9/7/20.
//

#ifndef CYCLONITE_GEOMETRY_H
#define CYCLONITE_GEOMETRY_H

#include "./vulkan/staging.h"
#include <cstdint>

namespace cyclonite {
class Geometry
{
public:
    Geometry(vulkan::Staging::AllocatedMemory&& vertices, vulkan::Staging::AllocatedMemory&& indices) noexcept;

    [[nodiscard]] auto id() const -> uint64_t { return id_; }

private:
    static std::atomic_uint64_t _lastGeometryId;

private:
    uint64_t id_;
    vulkan::Staging::AllocatedMemory vertices_;
    vulkan::Staging::AllocatedMemory indices_;
};
}

#endif // CYCLONITE_GEOMETRY_H
