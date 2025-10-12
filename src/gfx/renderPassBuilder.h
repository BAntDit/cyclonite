//
// Created by anton on 8/3/25.
//

#ifndef CYCLONITE_RENDERPASSBUILDER_H
#define CYCLONITE_RENDERPASSBUILDER_H

#include "core/resourceSharedRef.h"
#include "core/resourceUniqueRef.h"
#include "gfx/color.h"
#include "gfx/common.h"
#include "gfx/config.h"
#include <array>

namespace cyclonite::gfx {
class RenderPassBuilder
{
public:
    RenderPassBuilder() = default;

    auto setDevice(core::ResourceSharedRef deviceRef) -> RenderPassBuilder&;

    auto setResolution(uint32_t width, uint32_t height) -> RenderPassBuilder&;

    auto setDepthStencilAttachment(core::ResourceSharedRef textureRef,
                                   real depthClearValue = 1.0f,
                                   uint8_t stencilClearValue = 0) -> RenderPassBuilder&;

    auto setRenderWindow(core::ResourceSharedRef renderWindowRef,
                         gfx::Color clearColor = gfx::Color{},
                         real depthClearValue = 1.0f,
                         uint8_t stencilClearValue = 0) -> RenderPassBuilder&;

    auto addColorAttachment(core::ResourceSharedRef textureRef,
                            uint32_t mipLevel,
                            gfx::Color clearColor = gfx::Color{}) -> RenderPassBuilder&;

    auto build() -> core::ResourceUniqueRef;

private:
    core::ResourceSharedRef deviceRef_;
    core::ResourceSharedRef depthStencilTextureRef_;
    core::ResourceSharedRef renderWindowRef_;
    std::array<core::ResourceSharedRef, config_t::max_color_attachment_count_v> colorAttachmentRefs_;
    std::array<std::pair<uint16_t, uint16_t>, config_t::max_color_attachment_count_v> colorAttachmentSubresDescs_;
    std::array<gfx::Color, config_t::max_color_attachment_count_v> colorClearValues_;
    uint32_t colorAttachmentCount_;
    uint32_t width_;
    uint32_t height_;
    real depthClearValue_;
    uint8_t stencilClearValue_;
};
}

#endif // CYCLONITE_RENDERPASSBUILDER_H
