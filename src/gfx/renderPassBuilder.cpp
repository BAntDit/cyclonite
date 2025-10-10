//
// Created by anton on 8/9/25.
//

#include "renderPassBuilder.h"
#include "device.h"
#include "gfx/config.h"
#include "texture.h"
#include <cassert>
#include <format>
#include <stdexcept>

namespace cyclonite::gfx {
auto RenderPassBuilder::setDevice(core::ResourceSharedRef deviceRef) -> RenderPassBuilder&
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

auto RenderPassBuilder::setDepthStencilAttachment(core::ResourceSharedRef textureRef,
                                                  real depthClearValue /*= 1.0f*/,
                                                  uint8_t stencilClearValue /*= 0*/) -> RenderPassBuilder&
{
    depthClearValue_ = depthClearValue;
    stencilClearValue_ = stencilClearValue;

    assert(textureRef.valid());
    depthStencilTextureRef_ = textureRef;
    return *this;
}

auto RenderPassBuilder::addColorAttachment(core::ResourceSharedRef textureRef, uint32_t mipLevel) -> RenderPassBuilder&
{
    if (renderWindowRef_.valid()) {
        throw std::logic_error("render pass is not able to render into color attachments and render window at once");
    }

    if (colorAttachmentCount_ == deviceRef_.as<gfx::Device>().limits().maxColorAttachmentCount) {
        throw std::runtime_error(std::format("color attachment count must not exceed device limitation {}",
                                             deviceRef_.as<gfx::Device>().limits().maxColorAttachmentCount));
    }

    if (colorAttachmentCount_ == config_t::max_color_attachment_count_v) {
        throw std::runtime_error(std::format("color attachment count must not exceed configuration limit {}",
                                             config_t::max_color_attachment_count_v));
    }

    assert(textureRef.valid());
    assert(textureRef.as<gfx::Texture>().type() == TextureType::TEXTURE_2D);

    colorAttachmentRefs_[colorAttachmentCount_] = textureRef;
    colorAttachmentSubresDescs_[colorAttachmentCount_] = std::make_pair(static_cast<uint16_t>(mipLevel), uint16_t{ 0 });

    colorAttachmentCount_++;

    return *this;
}

auto RenderPassBuilder::setRenderWindow(core::ResourceSharedRef renderWindowRef) -> RenderPassBuilder&
{
    if (colorAttachmentCount_ > 0) {
        throw std::logic_error("render pass is not able to render into color attachments and render window at once");
    }
    renderWindowRef_ = renderWindowRef;
    return *this;
}

auto RenderPassBuilder::build() -> core::ResourceUniqueRef
{
    auto rpRef = core::ResourceUniqueRef{};

    if (colorAttachmentCount_ > 0) {
        rpRef = deviceRef_.as<type_traits::platform_implementation_t<gfx::Device>>().createRenderPassWithRTVs(
          depthStencilTextureRef_,
          colorAttachmentRefs_,
          colorAttachmentSubresDescs_,
          width_,
          height_,
          depthClearValue_,
          stencilClearValue_);
    } else {
        rpRef = deviceRef_.as<type_traits::platform_implementation_t<gfx::Device>>().createRenderPassWithRenderWindow(
          renderWindowRef_, depthClearValue_, stencilClearValue_);
    }

    return rpRef;
}
}