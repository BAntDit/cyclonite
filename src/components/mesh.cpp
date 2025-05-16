//
// Created by anton on 12/2/21.
//

#include "mesh.h"

namespace cyclonite::components {
Mesh::Mesh() noexcept
  : baseSubMesh_{ nullptr }
  , subMeshCount_{ 0 }
{
}

Mesh::Mesh(SubMesh* baseSubMesh, uint16_t subMeshCount) noexcept
  : baseSubMesh_{ baseSubMesh }
  , subMeshCount_{ subMeshCount }
{
}

auto Mesh::getSubMesh(uint16_t index) -> SubMesh&
{
    return *(baseSubMesh_ + index);
}

auto Mesh::getSubMesh(uint16_t index) const -> SubMesh const&
{
    return *(baseSubMesh_ + index);
}

auto Mesh::getSubMeshSetMemory() const -> std::pair<SubMesh*, size_t>
{
    return std::make_pair(baseSubMesh_, static_cast<size_t>(subMeshCount_));
}
}