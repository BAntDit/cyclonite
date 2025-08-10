//
// Created by anton on 8/3/25.
//

#ifndef CYCLONITE_RENDERPASSBUILDER_H
#define CYCLONITE_RENDERPASSBUILDER_H

#include "core/resourceRef.h"
#include <vector>

namespace cyclonite::gfx {
class RenderPassBuilder
{
public:
    RenderPassBuilder() = default;

    auto setDevice(core::ResourceRef deviceRef) -> RenderPassBuilder&;

    auto setDepthStencilAttachment(core::ResourceRef textureRef) -> RenderPassBuilder&;

    auto setSurface(core::ResourceRef surfaceRef) -> RenderPassBuilder&;

    auto addColorAttachment(core::ResourceRef textureRef) -> RenderPassBuilder&;

    auto build() -> core::ResourceRef;

private:
    core::ResourceRef deviceRef_;
    core::ResourceRef depthStencilTextureRef_;
    core::ResourceRef surfaceRef_;
    std::vector<core::ResourceRef> colorAttachmentRefs_;
};
}

#endif // CYCLONITE_RENDERPASSBUILDER_H
