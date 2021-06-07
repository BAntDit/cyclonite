//
// Created by bantdit on 1/7/20.
//

#include "baseRenderTarget.h"

namespace cyclonite {
BaseRenderTarget::BaseRenderTarget(uint32_t width,
                                   uint32_t height,
                                   size_t colorAttachmentCount /* = 1*/,
                                   bool hasDepthStencil /*= false*/)
  : extent_{}
  , hasDepthStencil_{ hasDepthStencil }
  , colorAttachmentCount_{ colorAttachmentCount }
  , frameBuffers_{}
  , outputSemantics_{}
{
    extent_.width = width;
    extent_.height = height;
}

auto BaseRenderTarget::getColorAttachment(size_t bufferIndex, size_t attachmentIndex) const -> vulkan::ImageView const&
{
    assert(attachmentIndex < colorAttachmentCount_);
    return frameBuffers_[bufferIndex].getColorAttachment(attachmentIndex);
}

auto BaseRenderTarget::getColorAttachment(size_t bufferIndex, RenderTargetOutputSemantic semantic) const
  -> vulkan::ImageView const&
{
    assert(hasAttachment(semantic));
    return frameBuffers_[bufferIndex].getColorAttachment(outputSemantics_.find(semantic)->second);
}

auto BaseRenderTarget::getDepthStencilAttachment(size_t bufferIndex) const -> vulkan::ImageView const&
{
    assert(hasDepthStencil_);
    return frameBuffers_[bufferIndex].getDepthStencilAttachment();
}

auto BaseRenderTarget::hasAttachment(RenderTargetOutputSemantic semantic) const -> bool
{
    return outputSemantics_.count(semantic) > 0;
}
}
