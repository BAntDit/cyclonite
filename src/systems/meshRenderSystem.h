//
// Created by anton on 8/10/26.
//

#ifndef CYCLONITE_MESH_RENDER_SYSTEM_H
#define CYCLONITE_MESH_RENDER_SYSTEM_H

#include "core/resourceSharedRef.h"

namespace cyclonite::systems {
class MeshRenderSystem
{
public:
    MeshRenderSystem() = default;

    void init(core::ResourceSharedRef const& deviceRef,
              uint32_t width,
              uint32_t height,
              core::ResourceSharedRef const& depthStencilTargetRef,
              core::ResourceSharedRef const& normalsTargetRefRef);

private:
    core::ResourceSharedRef deviceRef_;
    core::ResourceSharedRef passBindingSchemaRef_;
    core::ResourceSharedRef gBufferRenderPassRef_;
};
}

#endif // CYCLONITE_MESH_RENDER_SYSTEM_H