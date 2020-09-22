//
// Created by bantdit on 9/7/20.
//

#ifndef CYCLONITE_GEOMETRY_H
#define CYCLONITE_GEOMETRY_H

#include "./vulkan/staging.h"
#include "rawDataView.h"
#include "typedefs.h"
#include <cstdint>

namespace cyclonite {
class Geometry
{
public:
    Geometry(uint32_t vertexCount,
             uint32_t indexCount,
             vulkan::Staging::AllocatedMemory&& vertices,
             vulkan::Staging::AllocatedMemory&& indices) noexcept;

    Geometry(Geometry const&) = delete;

    Geometry(Geometry&&) = default;

    ~Geometry() = default;

    auto operator=(Geometry const&) -> Geometry& = delete;

    auto operator=(Geometry &&) -> Geometry& = default;

    [[nodiscard]] auto id() const -> uint64_t { return id_; }

    [[nodiscard]] auto vertexCount() const -> uint32_t { return vertexCount_; }

    [[nodiscard]] auto indexCount() const -> uint32_t { return indexCount_; }

    [[nodiscard]] auto vertices() const -> RawDataView<vertex_t>;

    [[nodiscard]] auto indices() const -> RawDataView<index_type_t>;

    [[nodiscard]] auto firstIndex() const -> uint32_t;

    [[nodiscard]] auto baseVertex() const -> uint32_t;

private:
    static std::atomic_uint64_t _lastGeometryId;

private:
    uint64_t id_;
    uint32_t vertexCount_;
    uint32_t indexCount_;
    vulkan::Staging::AllocatedMemory vertices_;
    vulkan::Staging::AllocatedMemory indices_;
};
}

#endif // CYCLONITE_GEOMETRY_H
