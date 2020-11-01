//
// Created by bantdit on 9/7/20.
//

#include "geometry.h"

namespace cyclonite {
std::atomic_uint64_t Geometry::_lastGeometryId = 0ULL;

Geometry::Geometry(uint32_t vertexCount,
                   uint32_t indexCount,
                   vulkan::Staging::AllocatedMemory&& vertices,
                   vulkan::Staging::AllocatedMemory&& indices) noexcept
  : id_{ ++_lastGeometryId }
  , vertexCount_{ vertexCount }
  , indexCount_{ indexCount }
  , vertices_{ std::move(vertices) }
  , indices_{ std::move(indices) }
{}

auto Geometry::vertices() const -> RawDataView<vertex_t>
{
    return RawDataView<vertex_t>{ vertices_.ptr(), 0, vertexCount_ };
}

auto Geometry::indices() const -> RawDataView<index_type_t>
{
    return RawDataView<index_type_t>{ indices_.ptr(), 0, indexCount_ };
}

auto Geometry::firstIndex() const -> uint32_t
{
    assert(indices_.offset() % sizeof(index_type_t) == 0);

    return static_cast<uint32_t>(indices_.offset() / sizeof(index_type_t));
}

auto Geometry::baseVertex() const -> uint32_t
{
    assert(vertices_.offset() % sizeof(vertex_t) == 0);

    return static_cast<uint32_t>(vertices_.offset() / sizeof(vertex_t));
}
}
