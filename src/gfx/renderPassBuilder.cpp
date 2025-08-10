//
// Created by anton on 8/9/25.
//

#include "renderPassBuilder.h"
#include "device.h"
#include <cassert>
#include <stdexcept>

namespace cyclonite::gfx {
auto RenderPassBuilder::setDevice(core::ResourceRef deviceRef) -> RenderPassBuilder&
{
    assert(deviceRef.valid());
    deviceRef_ = deviceRef;
    return *this;
}

auto RenderPassBuilder::setDepthStencilAttachment(core::ResourceRef textureRef) -> RenderPassBuilder&
{
    assert(textureRef.valid());
    depthStencilTextureRef_ = textureRef;
    return *this;
}

auto RenderPassBuilder::addColorAttachment(core::ResourceRef textureRef) -> RenderPassBuilder&
{
    if (surfaceRef_.valid()) {
        throw std::logic_error("render pass is not able to render into color attachments and surface at once");
    }

    if (colorAttachmentRefs_.empty()) { // first attachment
        auto maxColorAttachmentCount = deviceRef_.as<gfx::Device>().limits().maxColorAttachmentCount;
        colorAttachmentRefs_.reserve(maxColorAttachmentCount);
    }

    assert(colorAttachmentRefs_.size() < colorAttachmentRefs_.capacity());
    assert(textureRef.valid());
    colorAttachmentRefs_.push_back(textureRef);
    return *this;
}

auto RenderPassBuilder::setSurface(core::ResourceRef surfaceRef) -> RenderPassBuilder&
{
    if (!colorAttachmentRefs_.empty()) {
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