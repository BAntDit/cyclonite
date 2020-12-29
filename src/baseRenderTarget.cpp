//
// Created by bantdit on 1/7/20.
//

#include "baseRenderTarget.h"

namespace cyclonite::render {
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

auto BaseRenderTarget::getDepthStencilClearValue() const -> VkClearDepthStencilValue
{
    assert(hasDepthStencil_);

    return std::visit(
      [](auto&& clearValues) -> VkClearDepthStencilValue { return clearValues[clearValues.size() - 1].depthStencil; },
      clearValues_);
}

auto BaseRenderTarget::getColorAttachmentClearValue(RenderTargetOutputSemantic semantic) const -> VkClearColorValue
{
    assert(hasAttachment(semantic));

    auto idx = outputSemantics_.find(semantic)->second;

    return std::visit([&](auto&& clearValues) -> VkClearColorValue { return clearValues[idx].color; }, clearValues_);
}

auto BaseRenderTarget::getColorAttachmentClearValue(size_t attachmentIndex) const -> VkClearColorValue
{
    assert(attachmentIndex < colorAttachmentCount_);

    return std::visit([&](auto&& clearValues) -> VkClearColorValue { return clearValues[attachmentIndex].color; },
                      clearValues_);
}

auto BaseRenderTarget::getClearValues() const -> std::pair<size_t, VkClearValue const*>
{
    return std::visit(
      [](auto&& clearValues) -> std::pair<size_t, VkClearValue const*> {
          return std::make_pair(clearValues.size(), clearValues.data());
      },
      clearValues_);
}
}
