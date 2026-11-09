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

    auto setPassResolution(std::string_view passName, uint16_t width, uint16_t height) -> Compositor&;

    auto setPassCamera(std::string_view passName, enttx::Entity cameraEntity) -> Compositor&;

    auto addExternalTexture(std::string_view textureName, core::ResourceSharedRef const& textureRef) -> Compositor&;

    auto addPassDepth(std::string_view passName, gfx::Format format) -> Compositor&;

    auto setPassDepth(std::string_view passName, std::string_view srcPassName) -> Compositor&;

    auto addPassColorOutput(std::string_view passName,
                            std::string_view outputName,
                            gfx::TextureType textureType,
                            gfx::Format format,
                            uint16_t layer,
                            uint16_t mip) -> Compositor&;

    auto addPassInputTexture(std::string_view passName,
                             std::string_view srcPassName,
                             std::string_view passOutputTextureName,
                             gfx::Format format) -> Compositor&;

    auto addPassInputTexture(std::string_view passName,
                             std::string_view externalTextureName,
                             gfx::Format format) -> Compositor&;

    auto addPassInputFromDepth(std::string_view passName, std::string_view srcPassName) -> Compositor&;

    auto addPassConstant(std::string_view passName, std::string_view constantBufferName) -> Compositor&;

    auto makeComposition() -> RenderComposition;

private:
    struct texture_t
    {
        size_t textureIndex;
        gfx::Format format;
        bool external;
    };

    struct depth_t
    {
        size_t textureIndex;
        gfx::Format format;
    };

    struct rtv_t
    {
        size_t textureIndex;
        uint16_t layer;
        uint16_t mip;
        gfx::TextureType type;
        gfx::Format format;
    };

    struct pass_t
    {
        core::ResourceSharedRef passRef;
        enttx::Entity cameraEntity;
        uint16_t width;
        uint16_t height;
        RenderPassType type;
    };

    [[nodiscard]] auto getPass(std::string_view passName) -> pass_t&;

    core::ResourceSharedRef device_;
    boost::unordered_flat_map<std::string, pass_t> passes_;
    boost::unordered_flat_map<std::string, texture_t> textureInputs_;

    boost::unordered_flat_map<std::string,
                              std::tuple<std::string_view, gfx::TextureType, gfx::Format, uint16_t, uint16_t>>
      texturesToCreate_;

    std::vector<std::pair<std::string_view, std::string>> constantNames_;
    std::vector<std::tuple<std::string_view, std::string_view, std::string_view, gfx::Format>> inputsToCreate_;

    boost::unordered_flat_map<std::string_view, gfx::Format> depthToCreate_;
    boost::unordered_flat_map<std::string_view, std::string_view> depthToReuse_;

    std::vector<core::ResourceSharedRef> externalTextures_;
    std::vector<core::ResourceSharedRef> textures_;
};
}

#endif // CYCLONITE_COMPOSITION_COMPOSITOR_H
