//
// Created by bantdit on 2/11/20.
//

#ifndef CYCLONITE_MESH_H
#define CYCLONITE_MESH_H

#include "../vulkan/staging.h"

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
        SubMesh(size_t commandIdx)
          : commandIndex{ commandIdx }
        {}

        size_t commandIndex;
    };

private:
    std::vector<SubMesh> subMeshes;
};
}

#endif // CYCLONITE_MESH_H
