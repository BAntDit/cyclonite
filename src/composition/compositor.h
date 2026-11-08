//
// Created by anton on 11/8/26.
//

#ifndef CYCLONITE_COMPOSITION_COMPOSITOR_H
#define CYCLONITE_COMPOSITION_COMPOSITOR_H

#include "composition/common.h"
#include "renderComposition.h"

namespace cyclonite::composition {
class Compositor
{
public:
    Compositor(core::ResourceSharedRef const& device);

    auto addPass(std::string_view name, RenderPassType type) -> Compositor&;

    auto setPassResolution(uint32_t width, uint32_t height) -> Compositor&;

    auto setPassCamera(std::string_view passName, enttx::Entity cameraEntity) -> Compositor&;

    auto addExternalTexture(std::string_view name, core::ResourceSharedRef const& textureRef) -> Compositor&;

    auto addPassDepth(std::string_view passName, gfx::Format format) -> Compositor&;

    auto addPassColorOutput(std::string_view passName,
                            std::string_view outputName,
                            gfx::TextureType textureType,
                            gfx::Format format,
                            uint16_t layer,
                            uint16_t mip) -> Compositor&;

    auto addPassInputTexture(std::string_view passName,
                             std::string_view srcPassName,
                             std::string_view passOutputTextureName,
                             gfx::Format format);

    auto addPassInputTexture(std::string_view passName,
                             std::string_view externalTextureName,
                             gfx::Format format);

    auto addPassInputFromDepth(std::string_view passName, std::string_view srcPassName, gfx::Format format);

    auto addPassConstant(std::string_view passName, std::string_view constantBufferName) -> Compositor&;

    auto makeComposition() -> RenderComposition;
};
}

#endif // CYCLONITE_COMPOSITION_COMPOSITOR_H
