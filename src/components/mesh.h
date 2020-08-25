//
// Created by bantdit on 2/11/20.
//

#ifndef CYCLONITE_MESH_H
#define CYCLONITE_MESH_H

namespace cyclonite::components {
struct Mesh
{
public:
    struct SubMesh {};

    size_t firstSubMeshIndex;
    size_t subMeshCount;
};
}

#endif // CYCLONITE_MESH_H
