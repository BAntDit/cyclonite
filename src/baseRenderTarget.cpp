//
// Created by bantdit on 1/7/20.
//

#include "baseRenderTarget.h"

namespace cyclonite {
BaseRenderTarget::BaseRenderTarget(uint32_t width, uint32_t height)
  : extent_{}
  , colorAttachmentCount_{ 0 }
  , swapChainLength_{ 0 }
  , frontBufferIndex_{ 0 }
  , backBufferIndex_{ 0 }
  , frameBuffers_{}
  , outputSemantics_{}
  , hasDepthStencil_{ false }
{
    extent_.width = width;
    extent_.height = height;
}

auto BaseRenderTarget::getColorAttachment(size_t attachmentIndex) const -> vulkan::ImageView const&
{
    assert(attachmentIndex < colorAttachmentCount_);
    return frameBuffers_[frontBufferIndex_].getColorAttachment(attachmentIndex);
}

auto BaseRenderTarget::getColorAttachment(RenderTargetOutputSemantic semantic) const -> vulkan::ImageView const&
{
    assert(hasAttachment(semantic));
    return frameBuffers_[frontBufferIndex_].getColorAttachment(outputSemantics_.find(semantic)->second);
}

auto BaseRenderTarget::getDepthStencilAttachment() const -> vulkan::ImageView const&
{
    assert(hasDepthStencil_);
    return frameBuffers_[frontBufferIndex_].getDepthStencilAttachment();
}

auto BaseRenderTarget::hasAttachment(RenderTargetOutputSemantic semantic) const -> bool
{
    return outputSemantics_.count(semantic) > 0;
}
}
