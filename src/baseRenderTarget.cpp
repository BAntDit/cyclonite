//
// Created by bantdit on 1/7/20.
//

#include "baseRenderTarget.h"

namespace cyclonite {
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

void BaseRenderTarget::setDepthStencilClearValue(VkClearDepthStencilValue clearValue)
{
    assert(hasDepthStencil_);

    std::visit([&](auto&& clearValues) -> void { clearValues[clearValues.size() - 1].depthStencil = clearValue; },
               clearValues_);
}

void BaseRenderTarget::setColorAttachmentClearValue(RenderTargetOutputSemantic semantic, VkClearColorValue clearValue)
{
    assert(hasAttachment(semantic));

    auto idx = outputSemantics_.find(semantic)->second;

    std::visit([&](auto&& clearValues) -> void { clearValues[idx].color = clearValue; }, clearValues_);
}
}
