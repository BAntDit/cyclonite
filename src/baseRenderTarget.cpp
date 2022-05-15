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
  , clearValues_{}
  , frameBuffers_{}
  , outputSemantics_{}
{
    extent_.width = width;
    extent_.height = height;

    // just for now
    if (hasDepthStencil_) {
        clearValues_.reserve(colorAttachmentCount + 1);
        clearValues_.resize(colorAttachmentCount, VkClearValue{});

        auto depthClear = VkClearValue{};
        depthClear.depthStencil = VkClearDepthStencilValue{ 1.f, 0 };

        clearValues_.push_back(depthClear);
    } else {
        clearValues_.resize(colorAttachmentCount, VkClearValue{});
    }
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

auto BaseRenderTarget::getAttachmentIndex(RenderTargetOutputSemantic semantic) const -> size_t
{
    assert(hasAttachment(semantic));
    return outputSemantics_.at(semantic);
}

auto BaseRenderTarget::frameBuffer(size_t bufferIndex) const -> vulkan::FrameBuffer const&
{
    assert(bufferIndex < frameBuffers_.size());
    return frameBuffers_[bufferIndex];
}
}
