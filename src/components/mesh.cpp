//
// Created by anton on 11/1/26.
//

#include "mesh.h"

namespace cyclonite::components {
void Mesh::addSubMesh(core::ResourceSharedRef const& geometryRef, core::ResourceSharedRef const& materialRef)
{
    subMeshes_.emplace_back(geometryRef, materialRef);
}
}
