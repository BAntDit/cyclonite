//
// Created by anton on 11/8/26.
//

#include "renderComposition.h"

namespace cyclonite::composition {
RenderComposition::RenderComposition()
  : nameToPass_{}
  , passes_{}
  , nameToRenderTarget_{}
  , renderTargets_{}
  , nameToInput_{}
  , textureInputs_{}
  , nameToConstant_{}
  , constants_{}
  , externalTextures_{}
  , textures_{}
  , constantsBuffer_(constants_ring_buffer_size_v, std::byte{ 0 })
  , constantsRing_{ constantsBuffer_.data() } {};

auto RenderComposition::getPassId(std::string_view name) const -> uint64_t
{
    auto id = std::numeric_limits<uint64_t>::max();
    if (nameToPass_.contains(name.data())) {
        id = nameToPass_.at(name.data());
    }
    return id;
}

auto RenderComposition::getPass(uint64_t passId) const -> core::ResourceSharedRef const&
{
    assert(passes_.contains(passId));
    auto const& pass = passes_.at(passId);
    return pass.passRef;
}

auto RenderComposition::getPassCamera(uint64_t passId) const -> enttx::Entity
{
    assert(passes_.contains(passId));
    auto const& pass = passes_.at(passId);
    return pass.cameraEntity;
}

auto RenderComposition::getTextureId(std::string_view name) const -> uint32_t
{
    auto id = std::numeric_limits<uint32_t>::max();
    if (nameToInput_.contains(name.data())) {
        id = nameToInput_.at(name.data());
    }
    return id;
}

auto RenderComposition::getTexture(uint32_t textureId) const -> core::ResourceSharedRef
{
    assert(textureInputs_.contains(textureId));
    auto const& texture = textureInputs_.at(textureId);

    auto texRef = core::ResourceSharedRef{};

    if (texture.external && texture.textureIndex < externalTextures_.size()) {
        texRef = externalTextures_[texture.textureIndex];
    } else if (texture.textureIndex < textures_.size()) {
        texRef = textures_[texture.textureIndex];
    }

    return texRef;
}

auto RenderComposition::getPassConstantsId(std::string_view name) const -> uint32_t
{
    auto id = std::numeric_limits<uint32_t>::max();
    if (nameToConstant_.contains(name.data())) {
        id = nameToConstant_.at(name.data());
    }
    return id;
}

auto RenderComposition::getPassConstants(uint32_t constantsId) const -> pass_constants_t const&
{
    assert(constants_.contains(constantsId));
    return constants_.at(constantsId);
}

void RenderComposition::releaseConstantsMemory(uint64_t lastCompletedFrameIndex)
{
    while (!constantsRing_.empty()) {
        if (constantsRing_.pop([=](auto& frameIndex) -> bool { return frameIndex <= lastCompletedFrameIndex; }) == 0) {
            break;
        }
    }
}
}