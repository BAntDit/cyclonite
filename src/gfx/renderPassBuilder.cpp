//
// Created by anton on 8/9/25.
//

#include "renderPassBuilder.h"
#include "config.h"
#include "device.h"
#include "texture.h"
#include <cassert>
#include <format>
#include <stdexcept>

namespace cyclonite::gfx {
auto RenderPassBuilder::setDevice(core::ResourceRef deviceRef) -> RenderPassBuilder&
{
    assert(deviceRef.valid());
    deviceRef_ = deviceRef;
    return *this;
}

auto RenderPassBuilder::setResolution(uint32_t width, uint32_t height) -> RenderPassBuilder&
{
    width_ = width;
    height_ = height;
    return *this;
}

auto RenderPassBuilder::setDepthStencilAttachment(core::ResourceRef textureRef) -> RenderPassBuilder&
{
    assert(textureRef.valid());
    depthStencilTextureRef_ = textureRef;
    return *this;
}

auto RenderPassBuilder::addColorAttachment(core::ResourceRef textureRef, uint32_t mipLevel) -> RenderPassBuilder&
{
    if (surfaceRef_.valid()) {
        throw std::logic_error("render pass is not able to render into color attachments and surface at once");
    }

    if (colorAttachmentCount_ == deviceRef_.as<gfx::Device>().limits().maxColorAttachmentCount) {
        throw std::runtime_error(std::format("color attachment count must not exceed device limitation {}",
                                             deviceRef_.as<gfx::Device>().limits().maxColorAttachmentCount));
    }

    if (colorAttachmentCount_ == compile_time_config_t::max_color_attachment_count_v) {
        throw std::runtime_error(std::format("color attachment count must not exceed configuration limit {}",
                                             compile_time_config_t::max_color_attachment_count_v));
    }

    assert(textureRef.valid());
    assert(textureRef.as<gfx::Texture>().type() == TextureType::TEXTURE_2D);

    colorAttachmentRefs_[colorAttachmentCount_] = textureRef;
    colorAttachmentSubresDescs_[colorAttachmentCount_] = std::make_pair(static_cast<uint16_t>(mipLevel), uint16_t{ 0 });

    colorAttachmentCount_++;

    return *this;
}

auto RenderPassBuilder::setSurface(core::ResourceRef surfaceRef) -> RenderPassBuilder&
{
    if (colorAttachmentCount_ > 0) {
        throw std::logic_error("render pass is not able to render into color attachments and surface at once");
    }
    surfaceRef_ = surfaceRef;
    return *this;
}

auto RenderPassBuilder::build() -> core::ResourceRef
{
    // TODO::
}
}