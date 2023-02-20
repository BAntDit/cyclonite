//
// Created by bantdit on 1/7/20.
//
#pragma once
#ifndef CYCLONITE_BASERENDERTARGET_H
#define CYCLONITE_BASERENDERTARGET_H

#include "vulkan/frameBuffer.h"

namespace cyclonite {
class BaseRenderTarget
{
public:
    BaseRenderTarget(uint32_t width, uint32_t height, size_t colorAttachmentCount = 1, bool hasDepthStencil = false);

    BaseRenderTarget(BaseRenderTarget const&) = delete;

    BaseRenderTarget(BaseRenderTarget&&) = default;

    ~BaseRenderTarget() = default;

    auto operator=(BaseRenderTarget const&) -> BaseRenderTarget& = delete;

    auto operator=(BaseRenderTarget &&) -> BaseRenderTarget& = default;

    [[nodiscard]] auto width() const -> uint32_t { return extent_.width; }

    [[nodiscard]] auto height() const -> uint32_t { return extent_.height; }

    [[nodiscard]] auto frameBufferCount() const -> size_t { return frameBuffers_.size(); }

    [[nodiscard]] auto getColorAttachment(size_t bufferIndex, size_t attachmentIndex) const -> vulkan::ImageView const&;

    [[nodiscard]] auto getColorAttachment(size_t bufferIndex, RenderTargetOutputSemantic semantic) const
      -> vulkan::ImageView const&;

    [[nodiscard]] auto getDepthStencilAttachment(size_t bufferIndex) const -> vulkan::ImageView const&;

    [[nodiscard]] auto hasAttachment(RenderTargetOutputSemantic semantic) const -> bool;

    [[nodiscard]] auto hasDepthStencil() const -> bool { return hasDepthStencil_; }

    [[nodiscard]] auto frameBuffers() const -> std::vector<vulkan::FrameBuffer> const& { return frameBuffers_; }

    [[nodiscard]] auto frameBuffer(size_t bufferIndex) const -> vulkan::FrameBuffer const&;

    [[nodiscard]] auto getAttachmentIndex(RenderTargetOutputSemantic semantic) const -> size_t;

    [[nodiscard]] auto colorAttachmentCount() const -> size_t { return colorAttachmentCount_; }

    [[nodiscard]] auto clearValues() const -> std::vector<VkClearValue> const& { return clearValues_; }

    // TODO:: multisampling support
    [[nodiscard]] auto multisampleSampleCount() const -> uint32_t { return 1; }

private:
    VkExtent2D extent_;
    bool hasDepthStencil_;

protected:
    size_t colorAttachmentCount_;
    std::vector<VkClearValue> clearValues_;
    std::vector<vulkan::FrameBuffer> frameBuffers_;
    std::unordered_map<RenderTargetOutputSemantic, size_t> outputSemantics_;
};
}

#endif // CYCLONITE_BASERENDERTARGET_H
