//
// Created by bantdit on 11/3/19.
//

#ifndef CYCLONITE_RENDERTARGET_H
#define CYCLONITE_RENDERTARGET_H

#include "surface.h"

namespace cyclonite {
// TODO:: adds enum class outputs

class RenderTarget
{
public:
    // template<typename DepthStencilAttachment, typename... ColorAttachments>
    RenderTarget(VkRenderPass vkRenderPass);

    // template<typename DepthStencilAttachment, typename ColorAttachment>
    RenderTarget(VkRenderPass vkRenderPass, Surface& surface);

    RenderTarget(RenderTarget const&) = delete;

    RenderTarget(RenderTarget&&) = default;

    ~RenderTarget() = default;

    auto operator=(RenderTarget const&) -> RenderTarget& = delete;

    auto operator=(RenderTarget &&) -> RenderTarget& = default;

private:
    std::optional<Surface> surface_;
};
}

#endif // CYCLONITE_RENDERTARGET_H
