//
// Created by anton on 8/3/25.
//

#ifndef CYCLONITE_RENDERPASSBUILDER_H
#define CYCLONITE_RENDERPASSBUILDER_H

#include "config.h"
#include "core/resourceSharedRef.h"
#include "core/resourceUniqueRef.h"
#include "core/resourceWeakRef.h"
#include <array>

namespace cyclonite::gfx {
class RenderPassBuilder
{
public:
    RenderPassBuilder() = default;

    auto setDevice(core::ResourceSharedRef deviceRef) -> RenderPassBuilder&;

    auto setResolution(uint32_t width, uint32_t height) -> RenderPassBuilder&;

    auto setDepthStencilAttachment(core::ResourceSharedRef textureRef) -> RenderPassBuilder&;

    auto setRenderWindow(core::ResourceSharedRef renderWindowRef) -> RenderPassBuilder&;

    auto addColorAttachment(core::ResourceSharedRef textureRef, uint32_t mipLevel) -> RenderPassBuilder&;

    auto build() -> core::ResourceUniqueRef;

private:
    core::ResourceSharedRef deviceRef_;
    core::ResourceSharedRef depthStencilTextureRef_;
    core::ResourceSharedRef renderWindowRef_;
    std::array<core::ResourceSharedRef, config_t::max_color_attachment_count_v> colorAttachmentRefs_;
    std::array<std::pair<uint16_t, uint16_t>, config_t::max_color_attachment_count_v> colorAttachmentSubresDescs_;
    uint32_t colorAttachmentCount_;
    uint32_t width_;
    uint32_t height_;
};
}

#endif // CYCLONITE_RENDERPASSBUILDER_H
