//
// Created by bantdit on 2/11/20.
//

#ifndef CYCLONITE_MESH_H
#define CYCLONITE_MESH_H

#include "../vulkan/staging.h"
#include "geometry.h"

namespace cyclonite::systems {
class MeshSystem;
}

namespace cyclonite::components {
struct Mesh
{
public:
    friend class systems::MeshSystem;

    struct SubMesh
    {
        SubMesh(size_t commandIdx, std::shared_ptr<Geometry> const& geometry)
          : geometry{ geometry }
          , commandIndex{ commandIdx }
        {}

        std::shared_ptr<Geometry> geometry;
        size_t commandIndex;
    };

private:
    std::vector<SubMesh> subMeshes;
};
}

#endif // CYCLONITE_MESH_H
