//
// Created by bantdit on 9/7/20.
//

#include "geometry.h"

namespace cyclonite {
std::atomic_uint64_t Geometry::_lastGeometryId = 0ULL;

Geometry::Geometry(vulkan::Staging::AllocatedMemory&& vertices, vulkan::Staging::AllocatedMemory&& indices) noexcept
  : id_{ ++_lastGeometryId }
  , vertices_{ std::move(vertices) }
  , indices_{ std::move(indices) }
{}
}
