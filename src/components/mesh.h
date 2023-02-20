//
// Created by bantdit on 2/11/20.
//

#ifndef CYCLONITE_MESH_H
#define CYCLONITE_MESH_H

#include <cstddef>
#include <cstdint>
#include <utility>

namespace cyclonite::components {
struct SubMesh
{
    uint64_t geometryId;
    uint64_t materialId;
    uint32_t commandIndex;
};

struct Mesh
{
    Mesh() noexcept;

    Mesh(SubMesh* baseSubMesh, uint16_t subMeshCount) noexcept;

    auto getSubMesh(uint16_t index) -> SubMesh&;

    [[nodiscard]] auto getSubMesh(uint16_t index) const -> SubMesh const&;

    [[nodiscard]] auto getSubMeshCount() const -> uint16_t { return subMeshCount_; }

    [[nodiscard]] auto getSubMeshSetMemory() const -> std::pair<SubMesh*, size_t>;

private:
    SubMesh* baseSubMesh_;
    uint16_t subMeshCount_;
};
}

#endif // CYCLONITE_MESH_H
