//
// Created by anton on 8/3/25.
//

#ifndef CYCLONITE_RENDERPASSBUILDER_H
#define CYCLONITE_RENDERPASSBUILDER_H

#include "config.h"
#include "core/resourceRef.h"
#include <array>

namespace cyclonite::gfx {
class RenderPassBuilder
{
public:
    RenderPassBuilder() = default;

    auto setDevice(core::ResourceRef deviceRef) -> RenderPassBuilder&;

    auto setResolution(uint32_t width, uint32_t height) -> RenderPassBuilder&;

    auto setDepthStencilAttachment(core::ResourceRef textureRef) -> RenderPassBuilder&;

    auto setSurface(core::ResourceRef surfaceRef) -> RenderPassBuilder&;

    auto addColorAttachment(core::ResourceRef textureRef, uint32_t mipLevel) -> RenderPassBuilder&;

    auto build() -> core::ResourceRef;

private:
    core::ResourceRef deviceRef_;
    core::ResourceRef depthStencilTextureRef_;
    core::ResourceRef surfaceRef_;
    std::array<core::ResourceRef, compile_time_config_t::max_color_attachment_count_v> colorAttachmentRefs_;
    std::array<std::pair<uint16_t, uint16_t>, compile_time_config_t::max_color_attachment_count_v>
      colorAttachmentSubresDescs_;
    uint32_t colorAttachmentCount_;
    uint32_t width_;
    uint32_t height_;
};
}

#endif // CYCLONITE_RENDERPASSBUILDER_H
