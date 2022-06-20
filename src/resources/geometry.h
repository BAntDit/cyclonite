//
// Created by bantdit on 9/7/20.
//

#ifndef CYCLONITE_GEOMETRY_H
#define CYCLONITE_GEOMETRY_H

#include "buffers/bufferView.h"
#include "resource.h"
#include "staging.h"
#include "typedefs.h"
#include <cstdint>

namespace cyclonite::resources {
class Geometry : public Resource
{
public:
    Geometry(uint32_t vertexCount,
             uint32_t indexCount,
             resources::Staging::AllocatedMemory&& vertices,
             resources::Staging::AllocatedMemory&& indices) noexcept;

    [[nodiscard]] auto instance_tag() const -> ResourceTag const& override { return tag; }

    [[nodiscard]] auto vertexCount() const -> uint32_t { return vertexCount_; }

    [[nodiscard]] auto indexCount() const -> uint32_t { return indexCount_; }

    [[nodiscard]] auto firstIndex() const -> uint32_t;

    [[nodiscard]] auto baseVertex() const -> uint32_t;

    auto vertices() -> buffers::BufferView<vertex_t>;

    auto indices() -> buffers::BufferView<index_type_t>;

private:
    uint32_t vertexCount_;
    uint32_t indexCount_;
    resources::Staging::AllocatedMemory vertices_;
    resources::Staging::AllocatedMemory indices_;

private:
    static ResourceTag tag;

public:
    static auto type_tag_const() -> ResourceTag const& { return Geometry::tag; }
    static auto type_tag() -> ResourceTag& { return Geometry::tag; }
};
}

#endif // CYCLONITE_GEOMETRY_H
