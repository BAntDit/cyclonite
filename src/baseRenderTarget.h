//
// Created by bantdit on 1/7/20.
//

#ifndef CYCLONITE_BASERENDERTARGET_H
#define CYCLONITE_BASERENDERTARGET_H

#include "vulkan/frameBuffer.h"

namespace cyclonite {
enum class RenderTargetOutputSemantic
{
    UNDEFINED = 0,
    DEFAULT = 1,
    LINEAR_HDR_COLOR = 2,
    MIN_VALUE = UNDEFINED,
    MAX_VALUE = LINEAR_HDR_COLOR,
    COUNT = MAX_VALUE + 1
};

class BaseRenderTarget
{
public:
    BaseRenderTarget(uint32_t width, uint32_t height);

    BaseRenderTarget(BaseRenderTarget const&) = delete;

    BaseRenderTarget(BaseRenderTarget&&) = default;

    ~BaseRenderTarget() = default;

    auto operator=(BaseRenderTarget const&) -> BaseRenderTarget& = delete;

    auto operator=(BaseRenderTarget &&) -> BaseRenderTarget& = default;

    [[nodiscard]] auto width() const -> uint32_t { return extent_.width; }

    [[nodiscard]] auto height() const -> uint32_t { return extent_.height; }

    [[nodiscard]] auto frontBufferIndex() const -> size_t { return frontBufferIndex_; }

    [[nodiscard]] auto backBufferIndex() const -> size_t { return backBufferIndex_; }

    [[nodiscard]] auto swapChainLength() const -> size_t { return swapChainLength_; }

    [[nodiscard]] auto getColorAttachment(size_t attachmentIndex) const -> vulkan::ImageView const&;

    [[nodiscard]] auto getColorAttachment(RenderTargetOutputSemantic semantic) const -> vulkan::ImageView const&;

    [[nodiscard]] auto getDepthStencilAttachment() const -> vulkan::ImageView const&;

    [[nodiscard]] auto hasAttachment(RenderTargetOutputSemantic semantic) const -> bool;

    [[nodiscard]] auto hasDepthStencil() const -> bool { return hasDepthStencil_; }

    [[nodiscard]] auto frameBuffers() const -> std::vector<vulkan::FrameBuffer> const& { return frameBuffers_; }

    [[nodiscard]] auto colorAttachmentCount() const -> size_t { return colorAttachmentCount_; }

private:
    VkExtent2D extent_;

protected:
    size_t colorAttachmentCount_;
    size_t swapChainLength_;
    size_t frontBufferIndex_;
    size_t backBufferIndex_;
    std::vector<vulkan::FrameBuffer> frameBuffers_;
    std::unordered_map<RenderTargetOutputSemantic, size_t> outputSemantics_;
    bool hasDepthStencil_;
};
}

#endif // CYCLONITE_BASERENDERTARGET_H
