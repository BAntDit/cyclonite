//
// Created by anton on 11/1/26.
//

#ifndef CYCLONITE_COMPONENTS_MESH_H
#define CYCLONITE_COMPONENTS_MESH_H

#include "core/resourceSharedRef.h"
#include <vector>

namespace cyclonite::components {
class Mesh
{
    struct SubMesh
    {
        core::ResourceSharedRef geometryRef;
        core::ResourceSharedRef materialRef;
    };

public:
    Mesh() = default;

    void addSubMesh(core::ResourceSharedRef const& geometryRef, core::ResourceSharedRef const& materialRef);

    [[nodiscard]] auto subMeshes() const -> std::vector<SubMesh> const& { return subMeshes_; }

private:
    std::vector<SubMesh> subMeshes_;
};
}

#endif // CYCLONITE_COMPONENTS_MESH_H
